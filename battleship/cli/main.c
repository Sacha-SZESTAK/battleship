#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../engine/createGame.h"
#include "../engine/game.h"
#include "display.h"

/* ─── Codes ANSI ─────────────────────────────────────────────── */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define BLINK   "\033[5m"
#define RED     "\033[31m"
#define WHITE   "\033[37m"
#define RED_BG  "\033[41m"

#define ROWS 10
#define COLS 10

/* ─── Initialisation des grilles ────────────────────────────── */

static void initGrids(Jeu *jeu) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            jeu->grille[i][j]       = (Case){CASE_VIDE, -1};
            jeu->attackPlayer[i][j] = (Case){CASE_VIDE, -1};
            jeu->attackEnnemy[i][j] = (Case){CASE_VIDE, -1};
            jeu->enemyGrid[i][j]    = (Case){CASE_VIDE, -1};
        }
    }
}

/* ─── Point d'entrée ─────────────────────────────────────────── */

int main(void) {
    fflush(stdout);

    /* Redimensionner le terminal */
    printf("\033[8;30;130t");

    Jeu jeu;
    int choix  = 1;
    int quitter = 0;

    /* État initial */
    jeu.x          = 0;
    jeu.y          = 0;
    jeu.taille     = 5;
    jeu.horizontal = true;
    jeu.lost       = false;
    jeu.tour       = 1;
    jeu.end        = 0;
    jeu.isDebug    = false;
    strcpy(jeu.version, "1.0.0");

    resetVar(&jeu);
    initGrids(&jeu);

    while (!quitter) {
        clearScreen();
        printLogo(1);
        printf("Version : %s\n", jeu.version);
        printf("\n\n");

        printf("Menu Principal\n");
        printf("1) Créer une partie\n");
        printf("2) Règles\n");
        printf("3) Options\n");
        printf("4) Informations\n");
        printf("5) Quitter\n");

        if (jeu.isDebug)
            printf(WHITE RED_BG BLINK "\n /!\\ DEBUG ACTIVÉ /!\\ \n" RESET);

        int erreur = 1;
        while (erreur) {
            printf(">");
            if (scanf("%i", &choix) != 1) {
                printf("\a");
                printf(RED BOLD "Erreur : les caractères autres que 0-9 sont interdits.\n" RESET);
                choix = 0;
                while (getchar() != '\n');
                continue;
            }

            if (choix >= 1 && choix <= 5) {
                erreur = 0;
                switch (choix) {
                    case 1: createGame(&jeu);    break;
                    case 2: rules();             break;
                    case 3: settings(&jeu);      break;
                    case 4: informations(&jeu);  break;
                    case 5: quitter = 1;         break;
                }
            } else {
                printf("Erreur : choix invalide\n");
            }
        }
    }

    return 0;
}
