
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
#define BG_BLACK "\033[40m"
#define BOLD    "\033[1m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"


#define ROWS 10
#define COLS 10


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
            if (strcmp(jeu->attackPlayer[jeu->y][jeu->x], "    ") != 0) {
                printf("\a");
                continue;
            }
            jeu->isShooting = false;
            jeu->attackPlayer[jeu->y][jeu->x] = " ➕ ";
            restoreTerminal(&oldSettings);
            clearScreen();
            afficherPlateau(jeu);
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
                jeu->attackPlayer[jeu->y][jeu->x] = " ❌ ";
                jeu->enemyGrid[jeu->y][jeu->x] = "💥";
                printf("touché ! Vous rejouez.\n");
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackPlayer[jeu->y][jeu->x] = " 🌊 ";
        jeu->enemyGrid[jeu->y][jeu->x] = " ➕ ";
        printf("raté\n");
    }

    // j1Replay reflète si on a touché ou non
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




void temp(){
    clearScreen();
    printf("coulé\n");
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
                printf("Vous avez coulé le %s ennemi !\n", b->name);
            else
                printf("L'ennemi a coulé votre %s !\n", b->name);
            temp();
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
    } while (strcmp(jeu->attackEnnemy[iy][ix], "    ") != 0 && safety < 1000);

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
                jeu->attackEnnemy[iy][ix] = " ❌ ";
                jeu->grille[iy][ix] = "💥";
                printf(RED "L'ennemi vous a touché en %c%d ! Il rejoue.\n" RESET, 'A' + iy, ix + 1);
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackEnnemy[iy][ix] = " 🌊 ";
        printf("L'ennemi a raté en %c%d.\n", 'A' + iy, ix + 1);
    }

    // j2Replay reflète si l'IA a touché ou non
    jeu->j2Replay = hit;

    afficherPlateau(jeu);
    printf("Appuyez sur Entrée pour continuer...");
    getchar();
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
    if (jeu->end == 1)
        printf(GREEN BOLD "Félicitations, vous avez gagné !\n" RESET);
    else
        printf(RED BOLD "Vous avez perdu... L'ennemi a coulé tous vos bateaux.\n" RESET);

    getchar();
    getchar();
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




