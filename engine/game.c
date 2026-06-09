#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "createGame.h"
#include "game.h"

//réseau
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"


#define ROWS 10
#define COLS 10

/* ─── Tir joueur ────────────────────────────────────────────── */

int shootPlayer(Jeu *jeu) {
    int x = jeu->x;
    int y = jeu->y;

    /* Case déjà tirée */
    if (jeu->attackPlayer[y][x].etat != CASE_VIDE)
        return -1;

    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->enemyBoats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y     : b->y + t;

            if (x == bx && y == by) {
                b->touch[t] = 1;
                jeu->attackPlayer[y][x] = (Case){CASE_TOUCHE, i};
                jeu->enemyGrid[y][x]    = (Case){CASE_TOUCHE, i};
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackPlayer[y][x] = (Case){CASE_RATE, -1};
        jeu->shootRates++;  
        jeu->shootPlayed++;
    }

    jeu->j1Replay = hit;

    /* Vérifie si le bateau touché est coulé */
    if (hit) {
        int sunkIdx = testSunk(jeu);
        if (sunkIdx >= 0) return SHOOT_COULE;
        jeu->shootHits++;
        jeu->shootPlayed++;
        return SHOOT_TOUCHE;
    }
    return SHOOT_RATE;
}

/* ══════════════════════════════════════════════════════════════
   IA — utilitaires internes
   ══════════════════════════════════════════════════════════════ */

/* Mélange aléatoire de 4 directions */
static void shuffleDirs(int dx[4], int dy[4]) {
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int t;
        t = dx[i]; dx[i] = dx[j]; dx[j] = t;
        t = dy[i]; dy[i] = dy[j]; dy[j] = t;
    }
}

/* Enregistre une touche en mode destruction */
static void iaRegisterHit(Jeu *jeu, int ix, int iy) {
    jeu->iaHitX[jeu->iaHitCount] = ix;
    jeu->iaHitY[jeu->iaHitCount] = iy;
    jeu->iaHitCount++;
    jeu->iaMode = 1;
}

/* Remet l'IA en mode chasse après avoir coulé */
static void iaResetHunt(Jeu *jeu) {
    jeu->iaMode = 0;
    jeu->iaHitCount = 0;
}

/* ─── Logique commune après le tir ─────────────────────────── */

static int iaProcessShot(Jeu *jeu, int ix, int iy, int *outIx, int *outIy) {
    if (outIx) *outIx = ix;
    if (outIy) *outIy = iy;

    bool hit = false;

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->boats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y     : b->y + t;

            if (ix == bx && iy == by) {
                b->touch[t] = 1;
                jeu->attackEnnemy[iy][ix] = (Case){CASE_TOUCHE, i};
                jeu->grille[iy][ix]       = (Case){CASE_TOUCHE, i};
                hit = true;

                iaRegisterHit(jeu, ix, iy);

                /* Bateau coulé ? */
                bool sunk = true;
                for (int tt = 0; tt < b->size; tt++)
                    if (!b->touch[tt]) { sunk = false; break; }

                if (sunk) {
                    b->drowned = true;
                    iaResetHunt(jeu);
                }
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackEnnemy[iy][ix] = (Case){CASE_RATE, -1};
        jeu->grille[iy][ix]       = (Case){CASE_RATE, -1};
    }

    jeu->j2Replay = hit;

    if (hit) {
        int sunkIdx = testSunk(jeu);
        if (sunkIdx >= 0) return SHOOT_COULE;
        return SHOOT_TOUCHE;
    }
    return SHOOT_RATE;
}

/* ══════════════════════════════════════════════════════════════
   NIVEAU 1 — Intermédiaire
   Aléatoire pur + cible les 4 voisins après une touche
   ══════════════════════════════════════════════════════════════ */

static int iaShootIntermediaire(Jeu *jeu, int *outIx, int *outIy) {
    int ix, iy, found = 0;

    if (jeu->iaMode == 1 && jeu->iaHitCount > 0) {
        int dx[] = {1, -1, 0, 0};
        int dy[] = {0, 0, 1, -1};
        shuffleDirs(dx, dy);

        for (int d = 0; d < 4 && !found; d++) {
            int cx = jeu->iaHitX[0] + dx[d];
            int cy = jeu->iaHitY[0] + dy[d];
            if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS
                && jeu->attackEnnemy[cy][cx].etat == CASE_VIDE) {
                ix = cx; iy = cy; found = 1;
            }
        }
    }

    if (!found) {
        int safety = 0;
        do {
            ix = rand() % COLS;
            iy = rand() % ROWS;
            safety++;
        } while (jeu->attackEnnemy[iy][ix].etat != CASE_VIDE && safety < 1000);
    }

    return iaProcessShot(jeu, ix, iy, outIx, outIy);
}

/* ══════════════════════════════════════════════════════════════
   NIVEAU 2 — Difficile
   Chasse en damier + destruction orientée
   ══════════════════════════════════════════════════════════════ */

static int iaShootDifficile(Jeu *jeu, int *outIx, int *outIy) {
    int ix, iy, found = 0;

    if (jeu->iaMode == 1) {
        /* 2+ touches : on connaît l'orientation */
        if (jeu->iaHitCount >= 2) {
            int minX = jeu->iaHitX[0], maxX = jeu->iaHitX[0];
            int minY = jeu->iaHitY[0], maxY = jeu->iaHitY[0];
            for (int k = 1; k < jeu->iaHitCount; k++) {
                if (jeu->iaHitX[k] < minX) minX = jeu->iaHitX[k];
                if (jeu->iaHitX[k] > maxX) maxX = jeu->iaHitX[k];
                if (jeu->iaHitY[k] < minY) minY = jeu->iaHitY[k];
                if (jeu->iaHitY[k] > maxY) maxY = jeu->iaHitY[k];
            }
            int horizontal = (minY == maxY);
            int candidates[2][2] = {
                {horizontal ? minX - 1 : minX, horizontal ? minY : minY - 1},
                {horizontal ? maxX + 1 : maxX, horizontal ? maxY : maxY + 1}
            };
            for (int c = 0; c < 2 && !found; c++) {
                int cx = candidates[c][0], cy = candidates[c][1];
                if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS
                    && jeu->attackEnnemy[cy][cx].etat == CASE_VIDE) {
                    ix = cx; iy = cy; found = 1;
                }
            }
        }

        /* 1 touche ou bouts bloqués : 4 directions */
        if (!found) {
            int dx[] = {1, -1, 0, 0};
            int dy[] = {0, 0, 1, -1};
            shuffleDirs(dx, dy);
            for (int d = 0; d < 4 && !found; d++) {
                int cx = jeu->iaHitX[0] + dx[d];
                int cy = jeu->iaHitY[0] + dy[d];
                if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS
                    && jeu->attackEnnemy[cy][cx].etat == CASE_VIDE) {
                    ix = cx; iy = cy; found = 1;
                }
            }
        }
    }

    /* Chasse en damier */
    if (!found) {
        int safety = 0;
        do {
            ix = rand() % COLS;
            iy = rand() % ROWS;
            safety++;
        } while ((jeu->attackEnnemy[iy][ix].etat != CASE_VIDE
                 || (ix + iy) % 2 != 0) && safety < 1000);

        /* Damier épuisé : n'importe quelle case libre */
        if (safety >= 1000) {
            do {
                ix = rand() % COLS;
                iy = rand() % ROWS;
            } while (jeu->attackEnnemy[iy][ix].etat != CASE_VIDE);
        }
    }

    return iaProcessShot(jeu, ix, iy, outIx, outIy);
}

/* ══════════════════════════════════════════════════════════════
   NIVEAU 3 — Diabolique
   Carte de probabilités recalculée à chaque tir
   ══════════════════════════════════════════════════════════════ */

static void iaCalculerProbabilites(Jeu *jeu) {
    int map[ROWS][COLS];
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            map[y][x] = 0;

    /* Bateaux encore en vie */
    int tailles[5];
    int nb = 0;
    for (int i = 0; i < 5; i++)
        if (!jeu->boats[i].drowned)
            tailles[nb++] = jeu->boats[i].size;

    /* Compter les placements possibles pour chaque bateau */
    for (int b = 0; b < nb; b++) {
        int sz = tailles[b];

        /* Horizontal */
        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x <= COLS - sz; x++) {
                int ok = 1;
                for (int t = 0; t < sz && ok; t++)
                    if (jeu->attackEnnemy[y][x + t].etat != CASE_VIDE) ok = 0;
                if (ok)
                    for (int t = 0; t < sz; t++)
                        map[y][x + t]++;
            }
        }

        /* Vertical */
        for (int y = 0; y <= ROWS - sz; y++) {
            for (int x = 0; x < COLS; x++) {
                int ok = 1;
                for (int t = 0; t < sz && ok; t++)
                    if (jeu->attackEnnemy[y + t][x].etat != CASE_VIDE) ok = 0;
                if (ok)
                    for (int t = 0; t < sz; t++)
                        map[y + t][x]++;
            }
        }
    }

    /* Mode destruction : booster les cases adjacentes aux touches */
    if (jeu->iaMode == 1 && jeu->iaHitCount >= 1) {
        int dx[] = {1, -1, 0, 0};
        int dy[] = {0, 0, 1, -1};
        for (int h = 0; h < jeu->iaHitCount; h++) {
            for (int d = 0; d < 4; d++) {
                int cx = jeu->iaHitX[h] + dx[d];
                int cy = jeu->iaHitY[h] + dy[d];
                if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS
                    && jeu->attackEnnemy[cy][cx].etat == CASE_VIDE)
                    map[cy][cx] += 50;
            }
        }

        /* Orientation connue : booster les bouts de l'axe */
        if (jeu->iaHitCount >= 2) {
            int minX = jeu->iaHitX[0], maxX = jeu->iaHitX[0];
            int minY = jeu->iaHitY[0], maxY = jeu->iaHitY[0];
            for (int k = 1; k < jeu->iaHitCount; k++) {
                if (jeu->iaHitX[k] < minX) minX = jeu->iaHitX[k];
                if (jeu->iaHitX[k] > maxX) maxX = jeu->iaHitX[k];
                if (jeu->iaHitY[k] < minY) minY = jeu->iaHitY[k];
                if (jeu->iaHitY[k] > maxY) maxY = jeu->iaHitY[k];
            }
            int horizontal = (minY == maxY);
            int bouts[2][2] = {
                {horizontal ? minX - 1 : minX, horizontal ? minY : minY - 1},
                {horizontal ? maxX + 1 : maxX, horizontal ? maxY : maxY + 1}
            };
            for (int c = 0; c < 2; c++) {
                int cx = bouts[c][0], cy = bouts[c][1];
                if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS
                    && jeu->attackEnnemy[cy][cx].etat == CASE_VIDE)
                    map[cy][cx] += 100;
            }
        }
    }

    /* Copier dans la struct */
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            jeu->iaProbaMap[y][x] = map[y][x];
}

static int iaShootDiabolique(Jeu *jeu, int *outIx, int *outIy) {
    iaCalculerProbabilites(jeu);

    int ix = 0, iy = 0, maxProba = -1;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (jeu->iaProbaMap[y][x] > maxProba
                && jeu->attackEnnemy[y][x].etat == CASE_VIDE) {
                maxProba = jeu->iaProbaMap[y][x];
                ix = x; iy = y;
            }
        }
    }

    return iaProcessShot(jeu, ix, iy, outIx, outIy);
}

/* ══════════════════════════════════════════════════════════════
   DISPATCHER PRINCIPAL
   ══════════════════════════════════════════════════════════════ */

int iaShoot(Jeu *jeu, int *outIx, int *outIy) {
    switch (jeu->iaDifficulty) {
        case 1:  return iaShootIntermediaire(jeu, outIx, outIy);
        case 2:  return iaShootDifficile(jeu, outIx, outIy);
        case 3:  return iaShootDiabolique(jeu, outIx, outIy);
        default: return iaShootIntermediaire(jeu, outIx, outIy);
    }
}

/* ─── Naufrage ──────────────────────────────────────────────── */

int testSunk(Jeu *jeu) {
    /* tour=1 → on vient de tirer sur l'ennemi → on vérifie enemyBoats
       tour=0 → l'IA vient de tirer → on vérifie boats joueur        */
    Boat *boats = (jeu->tour == 1) ? jeu->enemyBoats : jeu->boats;

    for (int i = 0; i < 5; i++) {
        Boat *b = &boats[i];
        if (b->drowned) continue;

        int hits = 0;
        for (int t = 0; t < b->size; t++)
            if (b->touch[t] == 1) hits++;

        if (hits == b->size) {
            b->drowned = true;
            return i; /* index du bateau coulé */
        }
    }
    return -1; /* aucun coulé */
}

/* ─── Fin de partie ──────────────────────────────────────────── */

void testEnd(Jeu *jeu) {
    int enemyDrowned = 0;
    for (int i = 0; i < 5; i++)
        if (jeu->enemyBoats[i].drowned) enemyDrowned++;
    if (enemyDrowned == 5) { jeu->end = 1; return; }

    int playerDrowned = 0;
    for (int i = 0; i < 5; i++)
        if (jeu->boats[i].drowned) playerDrowned++;
    if (playerDrowned == 5) jeu->end = 2;
}

/* ─── Debug ──────────────────────────────────────────────────── */

void toggleDebug(Jeu *jeu) {
    jeu->isDebug = !jeu->isDebug;
}
