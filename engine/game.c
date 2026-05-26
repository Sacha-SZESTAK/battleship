#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "createGame.h"
#include "game.h"

#define ROWS 10
#define COLS 10

/* ─── Tir joueur ────────────────────────────────────────────── */

int shootPlayer(Jeu *jeu) {
    int x = jeu->x;
    int y = jeu->y;

    /* Case déjà tirée */
    if (jeu->attackPlayer[y][x].etat != CASE_VIDE)
        return -1;

    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->enemyBoats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y     : b->y + t;

            if (x == bx && y == by) {
                b->touch[t] = 1;
                jeu->attackPlayer[y][x] = (Case){CASE_TOUCHE, i};
                jeu->enemyGrid[y][x]    = (Case){CASE_TOUCHE, i};
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackPlayer[y][x] = (Case){CASE_RATE, -1};
    }

    jeu->j1Replay = hit;

    /* Vérifie si le bateau touché est coulé */
    if (hit) {
        int sunkIdx = testSunk(jeu);
        if (sunkIdx >= 0) return SHOOT_COULE;
        return SHOOT_TOUCHE;
    }
    return SHOOT_RATE;
}

/* ─── Tir IA ────────────────────────────────────────────────── */

int iaShoot(Jeu *jeu, int *outIx, int *outIy) {
    int ix, iy;
    int safety = 0;

    do {
        ix = rand() % COLS;
        iy = rand() % ROWS;
        safety++;
    } while (jeu->attackEnnemy[iy][ix].etat != CASE_VIDE && safety < 1000);

    if (outIx) *outIx = ix;
    if (outIy) *outIy = iy;

    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->boats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y     : b->y + t;
            if (jeu->isGodMode) {
                bx = ix;
                by = iy;
            }
            if (ix == bx && iy == by) {
                b->touch[t] = 1;
                jeu->attackEnnemy[iy][ix] = (Case){CASE_TOUCHE, i};
                jeu->grille[iy][ix]       = (Case){CASE_TOUCHE, i};
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackEnnemy[iy][ix] = (Case){CASE_RATE, -1};
        jeu->grille[iy][ix]       = (Case){CASE_RATE, -1};
    }

    jeu->j2Replay = hit;

    if (hit) {
        int sunkIdx = testSunk(jeu);
        if (sunkIdx >= 0) return SHOOT_COULE;
        return SHOOT_TOUCHE;
    }
    return SHOOT_RATE;
}

/* ─── Naufrage ──────────────────────────────────────────────── */

int testSunk(Jeu *jeu) {
    /* tour=1 → on vient de tirer sur l'ennemi → on vérifie enemyBoats
       tour=0 → l'IA vient de tirer → on vérifie boats joueur        */
    Boat *boats = (jeu->tour == 1) ? jeu->enemyBoats : jeu->boats;

    for (int i = 0; i < 5; i++) {
        Boat *b = &boats[i];
        if (b->drowned) continue;

        int hits = 0;
        for (int t = 0; t < b->size; t++)
            if (b->touch[t] == 1) hits++;

        if (hits == b->size) {
            b->drowned = true;
            return i; /* index du bateau coulé */
        }
    }
    return -1; /* aucun coulé */
}

/* ─── Fin de partie ──────────────────────────────────────────── */

void testEnd(Jeu *jeu) {
    int enemyDrowned = 0;
    for (int i = 0; i < 5; i++)
        if (jeu->enemyBoats[i].drowned) enemyDrowned++;
    if (enemyDrowned == 5) { jeu->end = 1; return; }

    int playerDrowned = 0;
    for (int i = 0; i < 5; i++)
        if (jeu->boats[i].drowned) playerDrowned++;
    if (playerDrowned == 5) jeu->end = 2;
}

/* ─── Debug ──────────────────────────────────────────────────── */

void toggleDebug(Jeu *jeu) {
    jeu->isDebug = !jeu->isDebug;
}
