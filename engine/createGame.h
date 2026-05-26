#ifndef CREATEGAME_H
#define CREATEGAME_H

#include <stdbool.h>
#include <termios.h>

/* ─── États d'une case ─────────────────────────────────────── */
typedef enum {
    CASE_VIDE,
    CASE_BATEAU,
    CASE_TOUCHE,
    CASE_RATE,
    CASE_COULE
} EtatCase;

/* ─── Case ─────────────────────────────────────────────────── */
typedef struct {
    EtatCase etat;
    int boatIndex; /* -1 si vide */
} Case;

/* ─── Bateau ────────────────────────────────────────────────── */
typedef struct {
    int x, y;
    int size;
    bool horizontal;
    bool placed;
    char name[20];
    bool drowned;
    int touch[5];
} Boat;

/* ─── État de jeu ───────────────────────────────────────────── */
typedef struct {
    Case grille[10][10];        /* plateau joueur            */
    Case attackPlayer[10][10];  /* tirs du joueur sur ennemi */
    Case enemyGrid[10][10];     /* plateau ennemi            */
    Case attackEnnemy[10][10];  /* tirs de l'ennemi          */

    int nbBateaux;
    int tour;        /* 1 = joueur, 0 = IA */
    bool j1Replay;
    bool j2Replay;

    /* curseur/placement */
    int x;
    int y;
    int taille;
    bool horizontal;
    char ch;
    int num;
    bool isplacement;

    Boat boats[5];
    Boat enemyBoats[5];

    /* flags d'affichage (interprétés uniquement par la couche CLI) */
    bool displayEnnemy;
    bool isShooting;

    /* options */
    bool isDebug;
    bool lost;
    bool isGodMode;
    /*
     * end : 0 = en cours, 1 = joueur gagne, 2 = joueur perd,
     *       3 = abandon
     */
    int end;

    char version[10];
} Jeu;

/* ─── Prototypes ────────────────────────────────────────────── */
void configureTerminal(struct termios *oldSettings);
void restoreTerminal(struct termios *oldSettings);

void resetVar(Jeu *jeu);
void generateOpponentBoard(Jeu *jeu);
bool canPlaceBoatAt(Jeu *jeu, int x, int y, int taille, bool horizontal);
int  getBoatSize(int selected);

/*
 * placerBateau : pose le bateau `boatIndex` aux coordonnées
 * actuelles du jeu (jeu->x, jeu->y, jeu->horizontal, jeu->taille).
 * Retourne  0 : OK
 *          -1 : placement invalide (case occupée ou hors grille)
 */
int placerBateau(Jeu *jeu, int boatIndex);

#endif /* CREATEGAME_H */
