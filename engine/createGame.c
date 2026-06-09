#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include "createGame.h"

#define ROWS 10
#define COLS 10

/* ─── Terminal ──────────────────────────────────────────────── */

void configureTerminal(struct termios *oldSettings) {
    struct termios newSettings;
    tcgetattr(STDIN_FILENO, oldSettings);
    newSettings = *oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
}

void restoreTerminal(struct termios *oldSettings) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldSettings);
}

/* ─── Utilitaires ───────────────────────────────────────────── */

int getBoatSize(int selected) {
    switch (selected) {
        case 0: return 5; /* Porte-avion        */
        case 1: return 4; /* Croiseur           */
        case 2: return 3; /* Contre-torpilleur  */
        case 3: return 3; /* Sous-marin         */
        case 4: return 2; /* Torpilleur         */
        default: return 2;
    }
}

bool canPlaceBoatAt(Jeu *jeu, int x, int y, int taille, bool horizontal) {
    for (int t = 0; t < taille; t++) {
        int bx = horizontal ? x + t : x;
        int by = horizontal ? y : y + t;

        if (bx < 0 || bx >= COLS || by < 0 || by >= ROWS)
            return false;
        if (jeu->grille[by][bx].etat == CASE_BATEAU)
            return false;
    }
    return true;
}

/* ─── Réinitialisation ──────────────────────────────────────── */

void resetVar(Jeu *jeu) {
    jeu->x          = 0;
    jeu->y          = 0;
    jeu->taille     = 5;
    jeu->horizontal = true;
    jeu->lost       = false;
    jeu->tour       = 1;
    jeu->end        = 0;
    jeu->isplacement = false;
    jeu->isShooting  = false;
    jeu->displayEnnemy = false;
    jeu->j1Replay   = false;
    jeu->j2Replay   = false;
/*    jeu->coupsJoues   = 0;
    jeu->coupsTouches = 0;
    jeu->coupsRates   = 0;
*/
    /* Champs réseau : on ne réinitialise pas gameMode/netPort/netIp/netFd
    car ils sont configurés dans les options et persistent entre parties.
    On remet seulement l'état de connexion à déconnecté. */
    jeu->netStatus = NET_DISCONNECTED;

    /* ─── Réinitialisation IA ─────────────────────────────────
    CRITIQUE : ces champs doivent être à zéro avant chaque
    partie, sinon iaShoot lit des valeurs indéfinies sinon segfault.
    iaDifficulty persiste (choisi dans les options). */
    jeu->iaMode     = 0;
    jeu->iaHitCount = 0;
    for (int k = 0; k < 100; k++) {
        jeu->iaHitX[k] = 0;
        jeu->iaHitY[k] = 0;
    }
    for (int r = 0; r < 10; r++)
        for (int c = 0; c < 10; c++)
            jeu->iaProbaMap[r][c] = 0;

    /* Noms des bateaux joueur */
    const char *names[5] = {
        "Porte-Avion", "Croiseur", "Contre-Torpilleur",
        "Sous-Marin",  "Torpilleur"
    };

    for (int i = 0; i < 5; i++) {
        strncpy(jeu->boats[i].name,      names[i], sizeof(jeu->boats[i].name) - 1);
        strncpy(jeu->enemyBoats[i].name, names[i], sizeof(jeu->enemyBoats[i].name) - 1);
        jeu->boats[i].name[sizeof(jeu->boats[i].name) - 1]           = '\0';
        jeu->enemyBoats[i].name[sizeof(jeu->enemyBoats[i].name) - 1] = '\0';

        jeu->boats[i].drowned  = false;
        jeu->boats[i].placed   = false;
        jeu->boats[i].x        = 0;
        jeu->boats[i].y        = 0;
        jeu->boats[i].size     = getBoatSize(i);
        jeu->enemyBoats[i].drowned = false;
        jeu->enemyBoats[i].placed  = false;
        jeu->enemyBoats[i].x      = 0;
        jeu->enemyBoats[i].y      = 0;
        jeu->enemyBoats[i].size   = getBoatSize(i);

        for (int j = 0; j < 5; j++) {
            jeu->boats[i].touch[j]      = 0;
            jeu->enemyBoats[i].touch[j] = 0;
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            jeu->grille[i][j]       = (Case){CASE_VIDE, -1};
            jeu->attackPlayer[i][j] = (Case){CASE_VIDE, -1};
            jeu->enemyGrid[i][j]    = (Case){CASE_VIDE, -1};
            jeu->attackEnnemy[i][j] = (Case){CASE_VIDE, -1};
        }
    }
}

/* ─── Placement du bateau joueur ────────────────────────────── */

/*
 * Pose le bateau `boatIndex` sur jeu->grille aux coordonnées
 * (jeu->x, jeu->y) avec l'orientation jeu->horizontal.
 * Retourne  0 : succès
 *          -1 : placement invalide
 */
int placerBateau(Jeu *jeu, int boatIndex) {
    if (!canPlaceBoatAt(jeu, jeu->x, jeu->y, jeu->taille, jeu->horizontal))
        return -1;

    Boat *b    = &jeu->boats[boatIndex];
    b->x       = jeu->x;
    b->y       = jeu->y;
    b->size    = jeu->taille;
    b->horizontal = jeu->horizontal;
    b->placed  = true;

    for (int t = 0; t < jeu->taille; t++) {
        int bx = jeu->horizontal ? jeu->x + t : jeu->x;
        int by = jeu->horizontal ? jeu->y : jeu->y + t;
        jeu->grille[by][bx] = (Case){CASE_BATEAU, boatIndex};
    }
    return 0;
}

/* ─── Génération du plateau adverse ────────────────────────── */

void generateOpponentBoard(Jeu *jeu) {
    srand(time(NULL));

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            jeu->enemyGrid[i][j] = (Case){CASE_VIDE, -1};

    for (int i = 0; i < 5; i++) {
        int taille  = getBoatSize(i);
        bool placed = false;
        int safety  = 0;

        while (!placed && safety < 1000) {
            safety++;
            int  x          = rand() % COLS;
            int  y          = rand() % ROWS;
            bool horizontal = rand() % 2;

            bool valid = true;
            for (int t = 0; t < taille; t++) {
                int bx = horizontal ? x + t : x;
                int by = horizontal ? y : y + t;
                if (bx >= COLS || by >= ROWS)             { valid = false; break; }
                if (jeu->enemyGrid[by][bx].etat == CASE_BATEAU) { valid = false; break; }
            }

            if (valid) {
                Boat *b    = &jeu->enemyBoats[i];
                b->x       = x;
                b->y       = y;
                b->size    = taille;
                b->horizontal = horizontal;
                b->placed  = true;
                b->drowned = false;
                for (int k = 0; k < 5; k++) b->touch[k] = 0;

                for (int t = 0; t < taille; t++) {
                    int bx = horizontal ? x + t : x;
                    int by = horizontal ? y : y + t;
                    jeu->enemyGrid[by][bx] = (Case){CASE_BATEAU, i};
                }
                placed = true;
            }
        }
        /* En cas d'échec (ne devrait pas arriver) on ignore silencieusement */
    }
}
