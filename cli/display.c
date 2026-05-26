#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include "display.h"
#include "../engine/createGame.h"
#include "../engine/game.h"

/* ─── Codes ANSI ─────────────────────────────────────────────── */
#define RESET     "\033[0m"
#define BOLD      "\033[1m"
#define BLINK     "\033[5m"
#define ITALIC    "\x1B[3m"
#define RED       "\033[31m"
#define GREEN_TXT "\033[32m"
#define YELLOW    "\033[33m"
#define BLUE      "\033[34m"
#define CYAN      "\033[36m"
#define WHITE     "\033[37m"
#define BG_BLACK  "\033[40m"
#define RED_BG    "\033[41m"
#define GREEN     "\033[42m"   /* fond vert  */
#define BLUE_BG   "\033[44m"

#define ROWS 10
#define COLS 10

/* ─── Écran ─────────────────────────────────────────────────── */

void clearScreen(void) {
    printf("\033[H\033[J");
}

/* ─── Logo ───────────────────────────────────────────────────── */

void printLogo(int option) {
    const char *lines[7] = {
        "██████╗  █████╗ ████████╗████████╗██╗     ███████╗███████╗██╗  ██╗██╗██████╗ \n",
        "██╔══██╗██╔══██╗╚══██╔══╝╚══██╔══╝██║     ██╔════╝██╔════╝██║  ██║██║██╔══██╗\n",
        "██████╔╝███████║   ██║      ██║   ██║     █████╗  ███████╗███████║██║██████╔╝\n",
        "██╔══██╗██╔══██║   ██║      ██║   ██║     ██╔══╝  ╚════██║██╔══██║██║██╔═══╝ \n",
        "██████╔╝██║  ██║   ██║      ██║   ███████╗███████╗███████║██║  ██║██║██║     \n",
        "╚═════╝ ╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝╚═╝     \n",
        "                                                                MacOS Edition\n"
    };

    if (option == 2) {
        printf(BLUE BOLD);
        for (int i = 0; i < 6; i++) printf("%s", lines[i]);
        printf(RESET);

    } else if (option == 1) {
        for (int i = 0; i < 6; i++) {
            printf(BLUE BOLD);
            for (int j = 0; j < i; j++) printf("%s", lines[j]);
            printf(CYAN BOLD BLINK "%s" RESET, lines[i]);
            usleep(200000);
            printf("\033[%dA", i + 1);
        }
        printf(BLUE BOLD);
        for (int i = 0; i < 6; i++) printf("%s", lines[i]);
        printf(ITALIC "%s" RESET, lines[6]);
    }
}

/* ─── Plateau ────────────────────────────────────────────────── */

void afficherPlateau(Jeu *jeu) {
    /* Titres */
    if (jeu->displayEnnemy)
        printf("╔════════════════════" BLINK WHITE RED_BG "plateau de jeu ENNEMI" RESET "════════════════════╗");
    else
        printf("╔════════════════════════plateau de jeu═══════════════════════╗");
    printf("  ");
    if (jeu->displayEnnemy)
        printf("╔══════════════════" BLINK WHITE RED_BG "plateau des tirs ENNEMI" RESET "════════════════════╗\n");
    else
        printf("╔══════════════════════plateau des tirs═══════════════════════╗\n");

    printf("║       1    2    3    4    5    6    7    8    9    10       ║");
    printf("  ");
    printf("║       1    2    3    4    5    6    7    8    9    10       ║\n");

    printf("║     ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐     ║");
    printf("  ");
    printf("║     ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐     ║\n");

    for (int i = 0; i < 10; i++) {
        /* ── Plateau joueur (gauche) ── */
        printf("║ %c>  │", 'A' + i);
        for (int j = 0; j < 10; j++) {
            /* Hologramme placement */
            bool isBateau = false;
            if (jeu->isplacement) {
                for (int t = 0; t < jeu->taille; t++) {
                    int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                    int by = jeu->horizontal ? jeu->y     : jeu->y + t;
                    if (bx >= COLS || by >= ROWS) continue;
                    if (by == i && bx == j) { isBateau = true; break; }
                }
            }

            if (isBateau && jeu->isplacement) {
                if      (jeu->num == 1) printf(GREEN  BLINK " 🚢 " RESET "│");
                else if (jeu->num == 2) printf(RED_BG BLINK " 🚢 " RESET "│");
                else if (jeu->num == 3) printf(BLUE_BG BLINK " 🚢 " RESET "│");
                else                    printf(GREEN  BLINK " 🚢 " RESET "│");
                continue;
            }

            Case c = jeu->displayEnnemy ? jeu->enemyGrid[i][j] : jeu->grille[i][j];
            switch (c.etat) {
                case CASE_BATEAU: printf(" 🚢 │"); break;
                case CASE_TOUCHE: printf(RED    " ❌ " RESET "│"); break;
                case CASE_RATE:   printf(BLUE   " ➕ " RESET "│"); break;
                case CASE_COULE:  printf(RED_BG " 💥 " RESET "│"); break;
                default:          printf(" 🌊 │"); break;
            }
        }

        printf("     ║  ║");

        /* ── Plateau des tirs (droite) ── */
        printf(" %c>  │", 'A' + i);
        for (int j = 0; j < 10; j++) {
            if (jeu->isShooting && i == jeu->y && j == jeu->x) {
                printf(RED BLINK " ➕ " RESET "│");
                continue;
            }
            Case c = jeu->displayEnnemy ? jeu->attackEnnemy[i][j] : jeu->attackPlayer[i][j];
            switch (c.etat) {
                case CASE_TOUCHE: printf(RED    " ❌ " RESET "│"); break;
                case CASE_RATE:   printf(BLUE   " 🌊 " RESET "│"); break;
                case CASE_COULE:  printf(RED_BG " 💥 " RESET "│"); break;
                default:          printf("    │"); break;
            }
        }
        printf("     ║\n");

        if (i < 9) {
            printf("║     ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤     ║");
            printf("  ");
            printf("║     ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤     ║\n");
        }
    }

    printf("║     └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘     ║");
    printf("  ");
    printf("║     └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘     ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝");
    printf("  ");
    printf("╚═════════════════════════════════════════════════════════════╝\n");
}

/* ─── Pages statiques ────────────────────────────────────────── */

void rules(void) {
    clearScreen();
    printf(RED "Lire attentivement les règles\n\n" RESET);

    printf(BLUE "1) Règle du jeu\n" RESET);
    printf("La bataille navale est un jeu à 2 joueurs en 1 contre 1.\n");
    printf("Ils s'opposent avec comme objectif de détruire la flotte navale adverse.\n");
    printf("Vous allez devoir placer 5 bateaux (porte-avion, croiseur, contre-torpilleur,\n");
    printf("sous-marin et torpilleur) en vertical ou horizontal.\n");
    printf("Après avoir placé vos bateaux vous pourrez démarrer la partie.\n");
    printf("Pour attaquer, sélectionnez une case : si vous touchez vous continuez,\n");
    printf("sinon c'est au tour de l'adversaire.\n\n");

    printf(BLUE "2) Placement sur le tableau\n" RESET);
    printf("Le bateau sélectionné s'affiche en couleur sur le plateau.\n");
    printf("W/X pour changer de bateau, ZQSD pour déplacer, C pour orienter, Entrée pour valider.\n\n");

    printf(BLUE "3) Types de bateaux\n" RESET);
    printf("Porte-avion       : 5 cases\n");
    printf("Croiseur          : 4 cases\n");
    printf("Contre-torpilleur : 3 cases\n");
    printf("Sous-marin        : 3 cases\n");
    printf("Torpilleur        : 2 cases\n");

    printf("\n\n Appuyez sur une touche pour continuer.\n>");
    getchar(); getchar();
}

void informations(Jeu *jeu) {
    clearScreen();
    printLogo(2);
    printf("Bataille Navale - Version %s\n\n", jeu->version);
    printf("Projet réalisé par Sacha SZESTAK dans le cadre du module de programmation C\n");
    printf("du semestre 2 à UniLaSalle Amiens.\n");
    printf("\nGithub : https://github.com/Sacha-SZESTAK/battleship\n");
    printf("\nCrédits : Sacha SZESTAK, Samuel MOATTI - I1 - 2026\n");
    printf("\n\n Appuyez sur une touche pour continuer.\n>");
    getchar(); getchar();
}

/* ─── Menu horizontal ────────────────────────────────────────── */

void displayHorizontalMenu(const char **options, int size, int selected,
                            int plateau, Jeu *jeu) {
    if (plateau) afficherPlateau(jeu);

    for (int i = 0; i < size; i++) {
        if (i == selected)
            printf(GREEN BOLD " %s " RESET, options[i]);
        else
            printf(" %s ", options[i]);
        printf("  ");
    }
    printf("\n");
    printf("\nAppuyez sur W/X pour naviguer et Entrée pour valider.\n");
}

/* ─── Placement interactif ───────────────────────────────────── */

int interactivePlacement(const char **options, int size, Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    char ch;
    int placedBoats[5] = {0, 0, 0, 0, 0};

    while (1) {
        clearScreen();

        /* Sauter les bateaux déjà posés */
        while (placedBoats[selected]) selected = (selected + 1) % size;

        switch (selected) {
            case 0: jeu->taille = 5; jeu->num = 1; break;
            case 1: jeu->taille = 4; jeu->num = 2; break;
            case 2: jeu->taille = 3; jeu->num = 3; break;
            case 3: jeu->taille = 3; jeu->num = 4; break;
            case 4: jeu->taille = 2; jeu->num = 5; break;
        }

        afficherPlateau(jeu);
        displayHorizontalMenu(options, size, selected, 0, jeu);

        read(STDIN_FILENO, &ch, 1);

        if (ch == 'w' || ch == 'W') {
            int orig = selected;
            do { selected = (selected - 1 + size) % size; }
            while (placedBoats[selected] && selected != orig);

        } else if (ch == 'x' || ch == 'X') {
            int orig = selected;
            do { selected = (selected + 1) % size; }
            while (placedBoats[selected] && selected != orig);

        } else if (ch == 'z' && jeu->y > 0)            jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1)        jeu->y++;
        else if (ch == 'q' && jeu->x > 0)               jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1)        jeu->x++;
        else if (ch == 'c' || ch == 'C')                jeu->horizontal = !jeu->horizontal;

        else if (ch == '\n') {
            /* Tente de poser le bateau via le moteur */
            if (placerBateau(jeu, selected) != 0) {
                printf("\a"); /* position invalide */
                continue;
            }

            placedBoats[selected] = 1;
            jeu->x = 0; jeu->y = 0; jeu->horizontal = true;

            int allPlaced = 1;
            for (int i = 0; i < size; i++) if (!placedBoats[i]) allPlaced = 0;
            if (allPlaced) break;
        }

        if ( jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if (!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }

    restoreTerminal(&oldSettings);
    return 0;
}

/* ─── Tir interactif ─────────────────────────────────────────── */

int interactiveShoot(Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    char ch;
    jeu->isShooting = true;

    while (1) {
        clearScreen();
        afficherPlateau(jeu);
        printf("Sélectionnez la case où vous souhaitez tirer (ZQSD + Entrée, Échap = pause)\n");

        read(STDIN_FILENO, &ch, 1);

        if      (ch == 'z' && jeu->y > 0)       jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0)        jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        if (jeu->x >= COLS) jeu->x = COLS - 1;
        if (jeu->y >= ROWS) jeu->y = ROWS - 1;
        if (jeu->x < 0)     jeu->x = 0;
        if (jeu->y < 0)     jeu->y = 0;

        if (ch == '\n') {
            if (jeu->attackPlayer[jeu->y][jeu->x].etat != CASE_VIDE) {
                printf("\a"); /* case déjà tirée */
                continue;
            }
            jeu->isShooting = false;
            restoreTerminal(&oldSettings);
            return 0;

        } else if (ch == 27) { /* Échap → pause */
            restoreTerminal(&oldSettings);
            int result = pauseMenu(jeu);
            if (result == -1) {
                jeu->isShooting = false;
                jeu->end = 3;
                return -1;
            }
            configureTerminal(&oldSettings);
        }
    }
}

/* ─── Menu pause ─────────────────────────────────────────────── */

int pauseMenu(Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    const char *options[] = { "Reprendre", "Options","Entrer un code", "Quitter la partie" };
    int nbOptions = 4;
    char ch;

    while (1) {
        clearScreen();
        printf(BLUE BOLD "=== " BLINK "PAUSE" RESET BLUE BOLD " ===\n\n" RESET);
        for (int i = 0; i < nbOptions; i++) {
            if (i == selected)
                printf(GREEN BOLD " > %s\n" RESET, options[i]);
            else
                printf("   %s\n", options[i]);
        }
        printf("\n[Z/S] naviguer   [Entrée] valider\n");

        read(STDIN_FILENO, &ch, 1);

        if      (ch == 'z' || ch == 'Z') selected = (selected - 1 + nbOptions) % nbOptions;
        else if (ch == 's' || ch == 'S') selected = (selected + 1) % nbOptions;
        else if (ch == '\n') {
            switch (selected) {
                case 0:
                    restoreTerminal(&oldSettings);
                    return 0;

                case 1:
                    restoreTerminal(&oldSettings);
                    settings(jeu);
                    configureTerminal(&oldSettings);
                    break;

                case 3: {
                    int confirmSelected = 1;
                    char confirmCh;
                    while (1) {
                        clearScreen();
                        printf(RED BOLD "Quitter la partie ?\n\n" RESET);
                        if (confirmSelected == 0)
                            printf(GREEN BOLD " > Oui\n" RESET "   Non\n");
                        else
                            printf("   Oui\n" GREEN BOLD " > Non\n" RESET);
                        printf("\n[Z/S] naviguer   [Entrée] valider\n");

                        read(STDIN_FILENO, &confirmCh, 1);
                        if      (confirmCh == 'z' || confirmCh == 'Z') confirmSelected = 0;
                        else if (confirmCh == 's' || confirmCh == 'S') confirmSelected = 1;
                        else if (confirmCh == 27) break;
                        else if (confirmCh == '\n') {
                            if (confirmSelected == 0) {
                                restoreTerminal(&oldSettings);
                                jeu->end = 3;
                                return -1;
                            } else { break; }
                        }
                    }
                    break;
                }
                case 2: {
                    restoreTerminal(&oldSettings);
                    // 1. On affiche le cadre complet
                    printf("╔═════════════════════════════ Entrez un code de triche : ═════════════════════════════╗\n");
                    printf("║                                                                                      ║\n");
                    printf("╚══════════════════════════════════════════════════════════════════════════════════════╝\n");

                    // 2. On remonte le curseur d'une ligne (vers le haut) et on le déplace de 2 colonnes vers la droite
                    // \033[1A : remonte d'une ligne
                    // \033[2C : décale de 2 colonnes à droite
                    printf("\033[2A\033[2C"); 
                    fflush(stdout); // Important pour forcer l'affichage avant la saisie

                    // 3. On récupère l'entrée
                    char code[50];
                    fgets(code, sizeof(code), stdin);
                    code[strcspn(code, "\n")] = 0;

                    // 4. On redescend le curseur sous le cadre pour ne pas corrompre l'affichage suivant
                    printf("\033[1B\r"); // On descend d'une ligne et on revient au début
                    if (strcmp(code, "SAMSAM") == 0) {
                        printf(GREEN "Code validé : mode invincibilité activé !\n" RESET);
                        jeu->isGodMode = true;
                        sleep(2);
                    } else if (strcmp(code, "HARCELEURPRIME") == 0) {
                        printf(GREEN "Code validé : tous les bateaux révélés !\n" RESET);
                        for (int i = 0; i < 5; i++) {
                            jeu->enemyBoats[i].drowned = true;
                            for (int y = 0; y < 10; y++) {
                                for (int x = 0; x < 10; x++) {
                                    if (jeu->enemyGrid[y][x].boatIndex == i) {
                                        jeu->attackPlayer[y][x].etat = CASE_COULE;
                                    }
                                }
                            }
                        }
                        sleep(2);
                    } else {
                        printf(RED "Code invalide.\n" RESET);
                        sleep(2);
                    }
                break;
                }

            }
        } else if (ch == 27) {
            restoreTerminal(&oldSettings);
            return 0;
        }
    }
}

/* ─── Paramètres ─────────────────────────────────────────────── */

int settings(Jeu *jeu) {
    int choix = 0;
    getchar();

    while (1) {
        clearScreen();
        printf(RED "===OPTIONS===\n\n" RESET);
        printf("     Option     État\n");
        printf("0) : Retour\n");
        printf("1) : Debug       %i\n", jeu->isDebug == 1 ? 1 : 0);
        printf(">");

        if (scanf("%i", &choix) != 1) {
            printf("\a");
            printf(RED BOLD "Erreur : caractères invalides.\n" RESET);
            choix = 0;
            while (getchar() != '\n');
            continue;
        }

        if (choix == 0) return 0;
        if (choix == 1) { toggleDebug(jeu); continue; }

        printf("Erreur : choix invalide\n");
    }
}

/* ─── Boucle principale de jeu ──────────────────────────────── */

void playGame(Jeu *jeu) {
    srand(time(NULL));
    clearScreen();

    jeu->j1Replay = false;
    jeu->j2Replay = false;

    while (jeu->end == 0) {

        /* ── Tour du joueur ── */
        jeu->tour = 1;
        do {
            clearScreen();
            afficherPlateau(jeu);
            printf("À vous de jouer.\n");
            if (jeu->j1Replay) printf(GREEN "Vous avez touché, vous rejouez !\n" RESET);

            /* debug : affiche le plateau ennemi si activé */
            if (jeu->isDebug) {
                jeu->displayEnnemy = true;
                afficherPlateau(jeu);
                jeu->displayEnnemy = false;
                getchar();
            }

            int shootRet = interactiveShoot(jeu);
            if (shootRet == -1) break; /* abandon */

            int result = shootPlayer(jeu);
            if (result == SHOOT_COULE) {
                /* Trouver le bateau coulé et l'annoncer */
                for (int i = 0; i < 5; i++) {
                    if (jeu->enemyBoats[i].drowned &&
                        jeu->attackPlayer[jeu->y][jeu->x].boatIndex == i) {
                        clearScreen();
                        printf("⚓ Vous avez coulé le %s ennemi !\n", jeu->enemyBoats[i].name);
                        printf("Appuyez sur Entrée pour continuer...\n");
                        getchar();
                        break;
                    }
                }
            }

            testEnd(jeu);
        } while (jeu->j1Replay && jeu->end == 0);

        if (jeu->end != 0) break;

        /* ── Tour de l'IA ── */
        jeu->tour = 0;
        do {
            int ix = 0, iy = 0;
            clearScreen();
            afficherPlateau(jeu);
            if (jeu->j2Replay) printf(YELLOW "L'ennemi a touché, il rejoue !\n" RESET);

            int result = iaShoot(jeu, &ix, &iy);

            printf(YELLOW "L'ennemi tire en %c%d...\n" RESET, 'A' + iy, ix + 1);
            sleep(1);

            if (result == SHOOT_TOUCHE || result == SHOOT_COULE) {
                printf(RED "L'ennemi vous a touché en %c%d !\n" RESET, 'A' + iy, ix + 1);
                if (result == SHOOT_COULE) {
                    for (int i = 0; i < 5; i++) {
                        if (jeu->boats[i].drowned) {
                            printf(RED "💥 L'ennemi a coulé votre %s !\n" RESET, jeu->boats[i].name);
                            printf("Appuyez sur Entrée pour continuer...\n");
                            getchar();
                            break;
                        }
                    }
                }
            } else {
                printf("L'ennemi a raté en %c%d.\n", 'A' + iy, ix + 1);
            }
            sleep(1);

            testEnd(jeu);
        } while (jeu->j2Replay && jeu->end == 0);
    }

    /* ── Écran de fin ── */
    clearScreen();
    if (jeu->end == 1) {
        printf(GREEN_TXT BLINK
               " ██████╗  █████╗  ██████╗ ███╗   ██╗███████╗\n"
               "██╔════╝ ██╔══██╗██╔════╝ ████╗  ██║██╔════╝\n"
               "██║  ███╗███████║██║  ███╗██╔██╗ ██║█████╗  \n"
               "██║   ██║██╔══██║██║   ██║██║╚██╗██║██╔══╝  \n"
               "╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║███████╗\n"
               " ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝\n" RESET);
        printf(GREEN BOLD "Félicitations, vous avez gagné !\n" RESET);
    } else {
        printf(RED BLINK
               "██████╗ ███████╗██████╗ ██████╗ ██╗   ██╗\n"
               "██╔══██╗██╔════╝██╔══██╗██╔══██╗██║   ██║\n"
               "██████╔╝█████╗  ██████╔╝██║  ██║██║   ██║\n"
               "██╔═══╝ ██╔══╝  ██╔══██╗██║  ██║██║   ██║\n"
               "██║     ███████╗██║  ██║██████╔╝╚██████╔╝\n"
               "╚═╝     ╚══════╝╚═╝  ╚═╝╚═════╝  ╚═════╝ \n" RESET);
        printf(RED BOLD "Vous avez perdu...\n" RESET);
    }
    printf("Appuyez sur une touche pour revenir au menu principal.\n>");
    getchar();
}

/* ─── Création de partie ─────────────────────────────────────── */

void createGame(Jeu *jeu) {
    resetVar(jeu);
    clearScreen();

    const char *options[] = {
        "Porte-Avion", "Croiseur", "Contre-torpilleur", "Sous-marin", "Torpilleur"
    };
    int size = 5;

    printf("Sélectionnez la position de vos bateaux (ZQSD + Entrée).\n");
    jeu->isplacement = true;
    interactivePlacement(options, size, jeu);
    jeu->isplacement = false;

    clearScreen();
    afficherPlateau(jeu);

    printf("Récapitulatif\n");
    for (int i = 0; i < 5; i++)
        printf("%s : X=%i Y=%i\n", jeu->boats[i].name, jeu->boats[i].x, jeu->boats[i].y);

    printf("Êtes-vous satisfait du placement ? 1: oui  2: non\n");

    int choix = 0, erreur = 1;
    while (erreur) {
        printf(">");
        if (scanf("%i", &choix) != 1) {
            printf("\a");
            printf(RED BOLD "Erreur : saisie invalide.\n" RESET);
            choix = 0;
            while (getchar() != '\n');
            continue;
        }
        if (choix == 1 || choix == 2) {
            erreur = 0;
            if (choix == 1) {
                printf("Placement du plateau adverse...\n");
                generateOpponentBoard(jeu);

                if (jeu->isDebug) {
                    printf("Récapitulatif des bateaux adverses\n");
                    for (int i = 0; i < 5; i++)
                        printf("%s : X=%i Y=%i\n",
                               jeu->enemyBoats[i].name,
                               jeu->enemyBoats[i].x, jeu->enemyBoats[i].y);
                    /* affichage debug du plateau ennemi */
                    jeu->displayEnnemy = true;
                    afficherPlateau(jeu);
                    jeu->displayEnnemy = false;
                    getchar();
                }

                getchar();
                playGame(jeu);
            } else {
                createGame(jeu); /* recommencer le placement */
            }
        } else {
            printf("Erreur : choix invalide\n");
        }
    }
}
