#ifndef GAME_H
#define GAME_H

#include "createGame.h"

/* ─── Codes retour de shootPlayer() ────────────────────────── */
typedef enum {
    SHOOT_RATE   = 0,
    SHOOT_TOUCHE = 1,
    SHOOT_COULE  = 2   /* touché + bateau entièrement coulé     */
} ShootResult;

/* ─── Logique de tir joueur ─────────────────────────────────── */

/*
 * Tire sur la case (jeu->x, jeu->y) du plateau ennemi.
 * Met à jour attackPlayer, enemyGrid, et les touch[] du bateau.
 * Retourne un ShootResult et positionne jeu->j1Replay.
 * Si la case a déjà été tirée, retourne -1 sans modifier l'état.
 */
int shootPlayer(Jeu *jeu);

/* ─── Logique de tir IA ──────────────────────────────────────── */

/*
 * L'IA choisit une case aléatoire non encore tirée, tire dessus.
 * Met à jour attackEnnemy, grille, et les touch[] du bateau joueur.
 * Positionne jeu->j2Replay.
 * Remplit `outIx` et `outIy` avec la case choisie (peut être NULL).
 * Retourne un ShootResult.
 */
int iaShoot(Jeu *jeu, int *outIx, int *outIy);

/* ─── Vérification naufrage ──────────────────────────────────── */

/*
 * Vérifie si le dernier tir a coulé un bateau.
 * tour=1 → vérifie les bateaux ennemis, tour=0 → bateaux joueur.
 * Retourne l'index du bateau coulé (0-4) ou -1 si aucun.
 */
int testSunk(Jeu *jeu);

/* ─── Vérification fin de partie ─────────────────────────────── */

/*
 * Met à jour jeu->end :
 *   0 = en cours, 1 = joueur gagne, 2 = joueur perd.
 */
void testEnd(Jeu *jeu);

/* ─── Paramètres ──────────────────────────────────────────────── */

/*
 * Bascule le mode debug.
 */
void toggleDebug(Jeu *jeu);

#endif /* GAME_H */
