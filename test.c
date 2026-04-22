#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>


#include "createGame.h"
#include "display.h"
#include "game.h"

#define RESET   "\033[0m"
#define GREEN   "\033[42m"
#define BLINK   "\033[5m"
#define BOLD    "\033[1m"

#define ROWS 10
#define COLS 10



void debug(Jeu *jeu){
    if(jeu->isDebug == true){
        jeu->displayEnnemy = true;
            afficherPlateau(jeu);
            jeu->displayEnnemy = false;
            getchar();
    }
}





/*
void clearScreen() {
    printf("\033[H\033[J");
}*/

/*typedef struct {
    char* grille[10][10];
    int nbBateaux;
    int tour;
    int x;
    int y;
    int taille;
    bool horizontal;
    char ch;

} Jeu;*/
/*
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
*/
// Affiche la grille avec le bateau “hologramme”
void afficherGrille(Jeu *jeu) {
    clearScreen();
    printf("   ");
    for(int c=0;c<COLS;c++) printf(" %d  ", c+1);
    printf("\n");

    for(int r=0;r<ROWS;r++){
        printf("%c ", 'A'+r);
        for(int c=0;c<COLS;c++){
            bool isBateau = false;
            // Vérifier si le bateau hologramme couvre cette case
            for(int t=0;t<jeu->taille;t++){
                int bx = c;
                int by = r;
                if(jeu->horizontal){
                    bx = jeu->x + t;
                    by = jeu->y;
                } else {
                    bx = jeu->x;
                    by = jeu->y + t;
                }
                if(by == r && bx == c) isBateau = true;
            }
            if(isBateau)
                printf(GREEN BLINK " 🚢 " RESET);
            else
                printf(" 🌊 ");
        }
        printf("\n");
    }
}

int selectPos(Jeu *jeu){
     while(1){
        //afficherGrille(jeu, jeu->x, jeu->y, jeu->taille, jeu->horizontal);
        afficherPlateau(jeu);
        read(STDIN_FILENO, &jeu->ch, 1);

        if(jeu->ch == 'p') break; // Quitter
        else if(jeu->ch == 'z' && jeu->y>0) jeu->y--;               // haut
        else if(jeu->ch == 's' && jeu->y<ROWS-1) jeu->y++;          // bas
        else if(jeu->ch == 'q' && jeu->x>0) jeu->x--;               // gauche
        else if(jeu->ch == 'd' && jeu->x<COLS-1) jeu->x++;          // droite
        else if(jeu->ch == 'c' || jeu->ch == 'C') jeu->horizontal = !jeu->horizontal; // changer orientation

        // Limiter le bateau à l'intérieur de la grille
        if(jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if(!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }
    return 0;
}
/*
int main() {
    Jeu jeu;
    jeu.x = 0;
    jeu.y = 0;
    jeu.taille = 5;
    jeu.horizontal = true;
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            jeu.grille[r][c] = "🌊";

    int x=0, y=0;               // Position du coin supérieur gauche du bateau
    bool horizontal = true;      // Horizontal par défaut
    char ch;
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    selectPos(&jeu);

    restoreTerminal(&oldSettings);
    printf("Placement terminé à %c%d en %s\n", 'A'+y, x+1, horizontal ? "horizontal" : "vertical");
    return 0;
}*/