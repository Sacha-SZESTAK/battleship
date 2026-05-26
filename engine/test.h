#ifndef TEST_H
#define TEST_H

#include "createGame.h"

/*
 * Déplace le curseur (jeu->x, jeu->y) en lisant les touches ZQSD.
 * Appuyer sur 'p' termine la saisie.
 * S'arrête aussi si le bateau dépasse les limites de la grille.
 * Ne fait aucun affichage.
 */
int selectPos(Jeu *jeu);

#endif /* TEST_H */
