#ifndef DISPLAY_H
#define DISPLAY_H

#include "../engine/createGame.h"
#include "../engine/game.h"

/* ─── Écran ─────────────────────────────────────────────────── */
void clearScreen(void);

/* ─── Logo animé ────────────────────────────────────────────── */
/* option 1 = animation, 2 = statique */
void printLogo(int option);

/* ─── Plateau de jeu ────────────────────────────────────────── */
void afficherPlateau(Jeu *jeu);

/* ─── Menus statiques ───────────────────────────────────────── */
void rules(void);
void informations(Jeu *jeu);

/* ─── Menu horizontal (sélection bateau) ───────────────────── */
void displayHorizontalMenu(const char **options, int size, int selected,
                            int plateau, Jeu *jeu);

/* ─── Placement interactif des bateaux ──────────────────────── */
/* Retourne 0 quand tous les bateaux ont été placés. */
int interactivePlacement(const char **options, int size, Jeu *jeu);

/* ─── Tir interactif ────────────────────────────────────────── */
/*
 * Déplace le curseur de tir avec ZQSD, Entrée pour tirer.
 * Retourne  0 : tir effectué (coordonnées dans jeu->x / jeu->y)
 *          -1 : abandon (menu pause → quitter)
 */
int interactiveShoot(Jeu *jeu);

/* ─── Menu pause ─────────────────────────────────────────────── */
/* Retourne 0 : reprendre, -1 : quitter */
int pauseMenu(Jeu *jeu);

/* ─── Menu paramètres ────────────────────────────────────────── */
int settings(Jeu *jeu);

/* ─── Boucle de jeu principale ──────────────────────────────── */
void playGame(Jeu *jeu);

/* ─── Flux de création de partie ────────────────────────────── */
void createGame(Jeu *jeu);

#endif /* DISPLAY_H */
