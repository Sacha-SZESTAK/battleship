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

/* ─── Mode de jeu ───────────────────────────────────────────── */
typedef enum {
    MODE_LOCAL = 0,
    MODE_LAN   = 1
} GameMode;

/* ─── Rôle réseau ───────────────────────────────────────────── */
typedef enum {
    ROLE_HOST   = 0,
    ROLE_CLIENT = 1
} NetRole;

/* ─── État de connexion ─────────────────────────────────────── */
typedef enum {
    NET_DISCONNECTED = 0,
    NET_CONNECTED    = 1,
    NET_ERROR        = -1
} NetStatus;

/* ─── État de jeu ───────────────────────────────────────────── */
typedef struct {
    Case grille[10][10];        /* plateau joueur            */
    Case attackPlayer[10][10];  /* tirs du joueur sur ennemi */
    Case enemyGrid[10][10];     /* plateau ennemi            */
    Case attackEnnemy[10][10];  /* tirs de l'ennemi          */

    int nbBateaux;
    int tour;        /* 1 = joueur local, 0 = IA ou adversaire réseau */
    bool j1Replay;
    bool j2Replay;

    /* curseur / placement */
    int  x;
    int  y;
    int  taille;
    bool horizontal;
    char ch;
    int  num;
    bool isplacement;

    Boat boats[5];
    Boat enemyBoats[5];

    /* flags d'affichage */
    bool displayEnnemy;
    bool isShooting;

    int iaHitX[100];
    int iaHitY[100];
    int iaHitCount;
    int iaMode;
    int iaDifficulty;
    int iaProbaMap[10][10]; // 10 au lieu de ROWS/COLS

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

    /* ─── Champs réseau LAN ─────────────────────────────────── */
    GameMode  gameMode;   /* MODE_LOCAL ou MODE_LAN            */
    NetRole   netRole;    /* ROLE_HOST ou ROLE_CLIENT          */
    NetStatus netStatus;  /* état de la connexion              */
    int       netFd;      /* socket de communication (-1 = off)*/
    int       netPort;    /* port TCP (configurable)           */
    char      netIp[64];  /* IP de l'hôte (côté client)       */
} Jeu;

/* ─── Prototypes ────────────────────────────────────────────── */
void configureTerminal(struct termios *oldSettings);
void restoreTerminal(struct termios *oldSettings);

void resetVar(Jeu *jeu);
void generateOpponentBoard(Jeu *jeu);
bool canPlaceBoatAt(Jeu *jeu, int x, int y, int taille, bool horizontal);
int  getBoatSize(int selected);
int  placerBateau(Jeu *jeu, int boatIndex);

#endif /* CREATEGAME_H */
