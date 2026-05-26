#include <unistd.h>
#include <stdbool.h>

#include "createGame.h"
#include "test.h"

#define ROWS 10
#define COLS 10

/*
 * Lit les touches ZQSD/C pour déplacer le curseur et changer
 * l'orientation. 'p' valide la position.
 * Tout affichage est à la charge de la couche CLI.
 */
int selectPos(Jeu *jeu) {
    while (1) {
        read(0, &jeu->ch, 1);

        if      (jeu->ch == 'p')                          break;
        else if (jeu->ch == 'z' && jeu->y > 0)            jeu->y--;
        else if (jeu->ch == 's' && jeu->y < ROWS - 1)     jeu->y++;
        else if (jeu->ch == 'q' && jeu->x > 0)            jeu->x--;
        else if (jeu->ch == 'd' && jeu->x < COLS - 1)     jeu->x++;
        else if (jeu->ch == 'c' || jeu->ch == 'C')        jeu->horizontal = !jeu->horizontal;

        /* Garder le bateau dans les limites */
        if ( jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if (!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }
    return 0;
}
