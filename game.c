#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>


//délcaration des fichiers 
#include "createGame.h"
#include "display.h"
#include "game.h"
#include "test.h"


// Codes ANSI pour les couleurs et le style
#define RED   "\033[31m"
#define RESET   "\033[0m"
#define GREEN   "\033[42m"
#define GREEN_TXT "\033[32m"
#define BG_BLACK "\033[40m"
#define BOLD    "\033[1m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"

#define BLINK   "\033[5m"


#define ROWS 10
#define COLS 10


int settings(Jeu *jeu){
    
    int choix = 0;


    getchar();
    int erreur = 1;
        while (erreur)
            {
                clearScreen();
                printf(RED"===OPTIONS===\n\n"RESET);
                printf("     Option     Etat\n");
                printf("0) : Retour\n");
                printf("1) : Debug       %i\n",jeu->isDebug);

                printf(">");


                if (scanf("%i", &choix) != 1)
                {
                    printf("\a");
                    printf(RED BOLD);
                    printf("Erreur CRITIQUE  💣 \n");
                    printf("Les caractères autres que 0-9 sont INTERDITS 💣💣💣💣💣💣.\n");
                    choix = 0;
                    printf(RESET);
                    

                    // vider le buffer
                    while (getchar() != '\n');

                    continue;
                }

                if (choix >= 0 && choix <= 1)
                {
                    erreur = 0;

                    switch(choix)
                    {
                        case 0:
                            //printf("ok");
                            erreur = 0;
                            return 0;
                            break;

                        case 1:
                            jeu->isDebug = !jeu->isDebug;
                            erreur = 1;
                            break;
                        
                    
                    }
                }
                else
                {
                    printf("Erreur : choix invalide\n");
                }
            }

return 0;
}

int interactiveShoot(Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    char ch;
    jeu->isShooting = true;

    while (1) {
        clearScreen();
        afficherPlateau(jeu);
        printf("Sélectionnez la case ou vous souhaitez tirer\n");
        read(STDIN_FILENO, &ch, 1);

        if (ch == 'z' && jeu->y > 0) jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0) jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        if (jeu->x >= COLS) jeu->x = COLS - 1;
        if (jeu->y >= ROWS) jeu->y = ROWS - 1;
        if (jeu->x < 0) jeu->x = 0;
        if (jeu->y < 0) jeu->y = 0;

        else if (ch == '\n') {
            // Empêcher de tirer sur une case déjà tirée
            if (jeu->attackPlayer[jeu->y][jeu->x].etat != CASE_VIDE) {
                printf("\a");
                continue;
            }
            jeu->isShooting = false;
            jeu->attackPlayer[jeu->y][jeu->x] = (Case){CASE_VIDE, -1};
            restoreTerminal(&oldSettings);
            clearScreen();
            afficherPlateau(jeu);
            return 0;
        }else if (ch == 27) {  // touche Échap
            restoreTerminal(&oldSettings);
            int result = pauseMenu(jeu);
            if (result == -1) {
            jeu->end = 3;  // code "abandon"
            return -1;
        }
    configureTerminal(&oldSettings); // reprend le mode raw
}
    }
}

int pauseMenu(Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    const char *options[] = {
        "Reprendre",
        "Options",
        "Quitter la partie"
    };
    int nbOptions = 3;
    char ch;

    while (1) {
        clearScreen();
        printf(BLUE BOLD "=== "BLINK "PAUSE" RESET BLUE BOLD " ===\n\n" RESET);

        for (int i = 0; i < nbOptions; i++) {
            if (i == selected)
                printf(GREEN BOLD " > %s\n" RESET, options[i]);
            else
                printf("   %s\n", options[i]);
        }

        printf("\n[Z/S] naviguer   [Entrée] valider\n");

        read(STDIN_FILENO, &ch, 1);

        if (ch == 'z' || ch == 'Z') selected = (selected - 1 + nbOptions) % nbOptions;
        else if (ch == 's' || ch == 'S') selected = (selected + 1) % nbOptions;
        else if (ch == '\n') {
            //restoreTerminal(&oldSettings);
            switch (selected) {
                case 0: return 0;  // reprendre
                case 1:
                    restoreTerminal(&oldSettings);
                    settings(jeu);   // votre fonction options existante
                    configureTerminal(&oldSettings);
                    break;
                case 2:
                    // Rester en mode raw pour la confirmation
                    clearScreen();
                    printf(RED BOLD "Quitter la partie ?\n\n" RESET);
                    printf(GREEN " > Oui\n" RESET);
                    printf("   Non\n");
                    printf("\n[Z/S] naviguer   [Entrée] valider\n");

                    int confirmSelected = 1; // Non par défaut (plus sûr)
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

                        if (confirmCh == 'z' || confirmCh == 'Z') confirmSelected = 0; // Oui
                        else if (confirmCh == 's' || confirmCh == 'S') confirmSelected = 1; // Non
                        else if (confirmCh == 27) break; // Échap → annule, retour pause
                        else if (confirmCh == '\n') {
                            if (confirmSelected == 0) {
                                restoreTerminal(&oldSettings);
                                jeu->end = 3;
                                return -1; // quitter confirmé
                            } else {
                                break; // Non → retour au menu pause
                            }
                        }
                    }
                    break; // retour à la boucle principale du pauseMenu
            }
        }
        // Échap ferme aussi la pause (reprend)
        else if (ch == 27) {
            restoreTerminal(&oldSettings);
            return 0;
        }
    }
}

void testShoot(Jeu *jeu) {
    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->enemyBoats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y : b->y + t;

            if (jeu->x == bx && jeu->y == by) {
                b->touch[t] = 1;
                jeu->attackPlayer[jeu->y][jeu->x] = (Case){CASE_TOUCHE, i};
                jeu->enemyGrid[jeu->y][jeu->x]    = (Case){CASE_TOUCHE, i};
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackPlayer[jeu->y][jeu->x] = (Case){CASE_RATE, -1};
    }

    jeu->j1Replay = hit;
    afficherPlateau(jeu);
    debug(jeu);
}


/*void testShoot(Jeu *jeu){
int h=0;
int v=0;
    for (int i=0; i<5; i++){
        if (jeu->x >= jeu->enemyBoats[i].x && jeu->x < jeu->enemyBoats[i].x+jeu->enemyBoats[i].size && jeu->y >= jeu->enemyBoats[i].y && jeu->y < jeu->enemyBoats[i].y+jeu->enemyBoats[i].size){
            printf("touché\n");

            if(jeu->enemyBoats[i].horizontal == true){
               h= jeu->x - jeu->enemyBoats[i].x;
               jeu->enemyBoats[i].touch[h] = 1;
            }else if(jeu->enemyBoats[i].horizontal == false){
                v= jeu->y - jeu->enemyBoats[i].y;
                jeu->enemyBoats[i].touch[v] = 1;
            }
            
            jeu->attackPlayer[jeu->y][jeu->x] = " ❌ ";
            jeu->enemyGrid[jeu->y][jeu->x] = "💥";
        }else {
                printf("raté\n");
                jeu->attackPlayer[jeu->y][jeu->x] = " 🌊 ";
                jeu->attackEnnemy[jeu->y][jeu->x] = " ➕ ";
            }

    }

*/
    /*for (int i=0; i<10;i++){
        for(int j=0; j<10;j++){
            if(strcmp(jeu->attackPlayer[i][j], " ➕ ") == 0 && strcmp(jeu->enemyGrid[i][j], "🚢") == 0){
                printf("touché\n");
                //jeu->attackEnnemy[i][j] = " 💥 ";
                jeu->enemyGrid[i][j] = "💥";
                jeu->attackPlayer[i][j] = " ❌ ";
                
            }else if(strcmp(jeu->attackPlayer[i][j], " ➕ ") == 0 && strcmp(jeu->enemyGrid[i][j], "🌊") == 0){
                printf("raté\n");
                jeu->attackPlayer[i][j] = " 🌊 ";
                jeu->attackEnnemy[i][j] = " ➕ ";
            }
        }
    }*/

 /*   afficherPlateau(jeu);
    //DEBUG
    debug(jeu);
    //DEBUG
}*/




void temp(const char *nomBateau, int tourJoueur){
    clearScreen();
    if (tourJoueur) {
        printf("⚓ Vous avez coulé le %s ennemi !\n", nomBateau);
    } else {
        printf("💥 L'ennemi a coulé votre %s !\n", nomBateau);
    }
    printf("Appuyez sur Entrée pour continuer...\n");
    getchar();
}


void testSunk(Jeu *jeu) {
    // tour=1 → joueur vient de tirer → on vérifie les bateaux ennemis
    // tour=0 → IA vient de tirer   → on vérifie les bateaux joueur
    Boat *boats = (jeu->tour == 1) ? jeu->enemyBoats : jeu->boats;

    for (int i = 0; i < 5; i++) {
        Boat *b = &boats[i];
        if (b->drowned) continue;

        int hits = 0;
        for (int t = 0; t < b->size; t++) {
            if (b->touch[t] == 1) hits++;
        }

        if (hits == b->size) {
            b->drowned = true;
            if (jeu->tour == 1)
                temp(b->name, 1);
            else
                temp(b->name, 0);
        }
    }
}


void testEnd(Jeu *jeu) {
    int enemyDrowned = 0;
    for (int i = 0; i < 5; i++) {
        if (jeu->enemyBoats[i].drowned) enemyDrowned++;
    }
    if (enemyDrowned == 5) {
        jeu->end = 1;
        return;
    }

    int playerDrowned = 0;
    for (int i = 0; i < 5; i++) {
        if (jeu->boats[i].drowned) playerDrowned++;
    }
    if (playerDrowned == 5) {
        jeu->end = 2;
    }
}


void iaShoot(Jeu *jeu) {
    int ix, iy;
    int safety = 0;

    do {
        ix = rand() % COLS;
        iy = rand() % ROWS;
        safety++;
    } while (jeu->attackEnnemy[iy][ix].etat != CASE_VIDE && safety < 1000);

    printf(YELLOW "L'ennemi tire en %c%d...\n" RESET, 'A' + iy, ix + 1);
    sleep(1);

    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->boats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y : b->y + t;

            if (ix == bx && iy == by) {
                b->touch[t] = 1;
                jeu->attackEnnemy[iy][ix] = (Case){CASE_TOUCHE, i};
                jeu->grille[iy][ix]       = (Case){CASE_TOUCHE, i};
                printf(RED "L'ennemi vous a touché en %c%d ! Il rejoue.\n" RESET, 'A' + iy, ix + 1);
                sleep(1);
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackEnnemy[iy][ix] = (Case){CASE_RATE, -1};
        jeu->grille[iy][ix]       = (Case){CASE_RATE, -1};
        printf("L'ennemi a raté en %c%d.\n", 'A' + iy, ix + 1);
        sleep(1);
    }

    jeu->j2Replay = hit;
    afficherPlateau(jeu);
}


void playGame(Jeu *jeu) {
    srand(time(NULL));
    clearScreen();

    // Initialiser les flags de replay à false
    jeu->j1Replay = false;
    jeu->j2Replay = false;

    while (jeu->end == 0) {

        // ══════════════ TOUR DU JOUEUR ══════════════
        jeu->tour = 1;

        do {
            afficherPlateau(jeu);
            printf("À vous de jouer.\n");
            if (jeu->j1Replay) printf(GREEN "Vous avez touché, vous rejouez !\n" RESET);
            debug(jeu);

            interactiveShoot(jeu);

            testShoot(jeu);
            testSunk(jeu);
            testEnd(jeu);
            //jeu->end = 1;

        } while (jeu->j1Replay == true && jeu->end == 0);

        if (jeu->end != 0) break;

        // ══════════════ TOUR DE L'IA ══════════════
        jeu->tour = 0;

        do {
            clearScreen();
            afficherPlateau(jeu);
            if (jeu->j2Replay) printf(YELLOW "L'ennemi a touché, il rejoue !\n" RESET);

            iaShoot(jeu);

            testSunk(jeu);
            testEnd(jeu);

        } while (jeu->j2Replay == true && jeu->end == 0);
    }

    clearScreen();
    if (jeu->end == 1){
        printf(GREEN_TXT BLINK" ██████╗  █████╗  ██████╗ ███╗   ██╗███████╗\n");
        printf("██╔════╝ ██╔══██╗██╔════╝ ████╗  ██║██╔════╝\n");
        printf("██║  ███╗███████║██║  ███╗██╔██╗ ██║█████╗  \n");
        printf("██║   ██║██╔══██║██║   ██║██║╚██╗██║██╔══╝  \n");
        printf("╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║███████╗\n");
        printf(" ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝\n"RESET);
        printf(GREEN BOLD "Félicitations, vous avez gagné !\n" RESET);
        
    }else{
        printf(RED BLINK "██████╗ ███████╗██████╗ ██████╗ ██╗   ██╗\n");
        printf("██╔══██╗██╔════╝██╔══██╗██╔══██╗██║   ██║\n");
        printf("██████╔╝█████╗  ██████╔╝██║  ██║██║   ██║\n");
        printf("██╔═══╝ ██╔══╝  ██╔══██╗██║  ██║██║   ██║\n");
        printf("██║     ███████╗██║  ██║██████╔╝╚██████╔╝\n");
        printf("╚═╝     ╚══════╝╚═╝  ╚═╝╚═════╝  ╚═════╝ \n"RESET);    
        printf(RED BOLD "Vous avez perdu... L'ennemi a coulé tous vos bateaux.\n" RESET);

        
    }
    printf("Appuyez sur une touche pour revenir au menu principal.\n");
    printf(">");
    getchar();
    //getchar();
}




/*void testSunk(Jeu *jeu){
    //tour = 0 -> joueur 
    //tour = 1 -> ennemi
    int posX = 0;
    int posY = 0;
    int test = 0;
    if (jeu->tour ==1){
    //printf("test");
        for(int i=0;i<5;i++){
            
            if(jeu->boats[i].horizontal == true){
                //printf("%s : %i",jeu->boats[i].name , jeu->boats->horizontal);
                for(int j = 0; j<jeu->boats->size; j++){
                    if(strcmp(jeu->enemyGrid[jeu->boats[i].y][jeu->boats[i].x+j], "💥") == 0){
                        
                        test++;
                    }
                }
                if (test == jeu->boats[i].size){
                    printf("coulé");
                    jeu->boats[i].drowned = true;
                }
            } 
            jeu->enemyBoats->drowned = true;
        }
    }
}*/