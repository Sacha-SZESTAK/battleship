#ifndef CREATEGAME_H
#define CREATEGAME_H

//#include "display.h"

typedef struct {
    int x, y;        // position du coin supérieur gauche
    int size;        // taille du bateau
    bool horizontal; // orientation
    bool placed;     // vrai si le bateau est déjà posé
    char name[20];   //nom du bateau
    bool drowned;
    int touch[5];
} Boat;

typedef struct {
    char* grille[10][10];
    char* attackPlayer[10][10];

    char *enemyGrid[10][10];
    char* attackEnnemy[10][10];
    int nbBateaux;
    int tour;

    int x;
    int y;
    int taille;
    bool horizontal;
    char ch;
    int num;
    bool isplacement;
    Boat boats[5]; // bateaux joueur
    Boat enemyBoats[5];  //bateaux ennemis
    bool displayEnnemy;
    bool isShooting;
    bool isDebug;
    bool lost;
    int end;
} Jeu;

void createGame(Jeu *jeu);

void restoreTerminal(struct termios *oldSettings);

void configureTerminal(struct termios *oldSettings);


#endif