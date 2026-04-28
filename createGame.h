#ifndef CREATEGAME_H
#define CREATEGAME_H

#include <stdbool.h>

// États possibles d'une case
typedef enum {
    CASE_VIDE,
    CASE_BATEAU,
    CASE_TOUCHE,
    CASE_RATE,
    CASE_COULE
} EtatCase;

// Structure d'une case
typedef struct {
    EtatCase etat;
    int boatIndex; // index du bateau sur cette case (-1 si vide)
} Case;

typedef struct {
    int x, y;
    int size;
    bool horizontal;
    bool placed;
    char name[20];
    bool drowned;
    int touch[5];
} Boat;

typedef struct {
    Case grille[10][10];        // plateau joueur
    Case attackPlayer[10][10];  // tirs du joueur sur l'ennemi
    Case enemyGrid[10][10];     // plateau ennemi
    Case attackEnnemy[10][10];  // tirs de l'ennemi sur le joueur
    int nbBateaux;
    int tour;
    bool j1Replay;
    bool j2Replay;
    int x;
    int y;
    int taille;
    bool horizontal;
    char ch;
    int num;
    bool isplacement;
    Boat boats[5];
    Boat enemyBoats[5];
    bool displayEnnemy;
    bool isShooting;
    bool isDebug;
    bool lost;
    int end;
} Jeu;

void createGame(Jeu *jeu);
void restoreTerminal(struct termios *oldSettings);
void configureTerminal(struct termios *oldSettings);
void resetVar(Jeu *jeu);

#endif