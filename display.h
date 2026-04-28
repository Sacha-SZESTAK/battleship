#ifndef DISPLAY_H
#define DISPLAY_H

#include "createGame.h"

void printLogo(int option);

void clearScreen();

void afficherPlateau(Jeu *jeu);

void displayHorizontalMenu(const char **options, int size, int selected, int plateau, Jeu *jeu);

//int interactiveMenu(const char **options, int size, int plateau, Jeu *jeu);

int interactivePlacement(const char **options, int size, Jeu *jeu);

bool canPlaceBoat(Jeu *jeu);

#endif