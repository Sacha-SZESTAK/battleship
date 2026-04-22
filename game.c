#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>




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


#define ROWS 10
#define COLS 10



/*int interactiveShoot(Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    char ch;

    // Tableau pour savoir quels bateaux ont été posés (0 = non posé, 1 = posé)
    int placedBoats[5] = {0, 0, 0, 0, 0};

    while (1) {
        clearScreen();

       

        // Affichage plateau + menu
        afficherPlateau(jeu);
        

        // Lecture d'une touche
        read(STDIN_FILENO, &ch, 1);

    

        // --- Déplacement ---
        if (ch == 'z' && jeu->y > 0) jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0) jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        // --- Rotation ---
        //else if (ch == 'c' || ch == 'C') jeu->horizontal = !jeu->horizontal;

        //Ensuite, dans ta boucle de placement, remplace le code qui posait le bateau par :

        // --- Placement final ---
        //else if (ch == '\n') {
            

            int idx = selected; // bateau sélectionné

            Boat *b = &jeu->boats[idx];
            b->x = jeu->x;
            b->y = jeu->y;
            b->size = jeu->taille;
            b->horizontal = jeu->horizontal;
            b->placed = true;

            // Placer le bateau
            // for (int t = 0; t < jeu->taille; t++) {
                // int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                // int by = jeu->horizontal ? jeu->y : jeu->y + t;
                jeu->grille[jeu->y][jeu->x] = "╳";
            // }

            // Marquer le bateau comme posé
            

            // Réinitialiser position et orientation pour le prochain bateau
            

            
        //}

        // --- Limites ---
        if (jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if (!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }

    restoreTerminal(&oldSettings);
    return 0;
}*/


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

        // déplacement
        if (ch == 'z' && jeu->y > 0) jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0) jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        // Limites
        if (jeu->x >= COLS) jeu->x = COLS - 1;
        if (jeu->y >= ROWS) jeu->y = ROWS - 1;
        if (jeu->x < 0) jeu->x = 0;
        if (jeu->y < 0) jeu->y = 0;

        // validation tir
        else if (ch == '\n') {
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
                printf("touché\n");
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackPlayer[jeu->y][jeu->x] = " 🌊 ";
        jeu->enemyGrid[jeu->y][jeu->x] = "➕";   // ← ajout du 1er
        printf("raté\n");
    }

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
    // On ne teste que si c'est le tour de l'ennemi
    if (jeu->tour != 1) return;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->enemyBoats[i];

        // Skip si déjà coulé
        if (b->drowned) continue;

        int hits = 0;

        // Vérifier chaque case du bateau
        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y : b->y + t;

            //if (strcmp(jeu->enemyGrid[by][bx], "💥") == 0) {
            //    hits++;
            //}

            if (jeu->enemyBoats[i].touch[t] == 1) {
                hits++;
            }
        }

        // Si toutes les cases touchées, bateau coulé
        if (hits == b->size) {
            b->drowned = true;
            printf("%s coulé !\n", b->name);
            temp();

        }
    }
}


void testEnd(Jeu *jeu){
int isDrowned = 0;
    for(int i=0; i<5; i++){
        if (jeu->enemyBoats[i].drowned == 1){
            isDrowned++;
        }
    }
    if (isDrowned == 5){
        jeu->end = 1;
    }

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

void playGame(Jeu *jeu){

    clearScreen();
    
    while(jeu->end == 0){
    
        afficherPlateau(jeu);
        
        printf("A vous de jouer.\n");
        printf("Selectionnez la case à tirer\n");

        //DEBUG
        debug(jeu);
        //DEBUG

        interactiveShoot(jeu);
        getchar();

        testShoot(jeu);

        testSunk(jeu);
        //jeu->tour = 0;

        testEnd(jeu);

    }

    clearScreen();
    printf("Fin du jeu");
    getchar();
    getchar();
    //bool test = true;
    //printf("jeu %hhd",test);
    //printf("%i",jeu->isShooting);
    //scanf("%i",&test);
    
}



