#ifndef SERVER_H
#define SERVER_H

/*
 * server.h dépend de createGame.h pour le type Jeu.
 * Tous les types (NetRole, NetStatus, GameMode) sont définis
 * dans createGame.h pour éviter toute dépendance circulaire.
 */
#include "createGame.h"

/* ─── Protocole réseau ──────────────────────────────────────────
 *
 * Chaque message échangé entre hôte et client est une ligne
 * terminée par '\n' de la forme :
 *
 *   TYPE:payload\n
 *
 * Types définis :
 *   HELLO        → handshake initial (client → hôte)
 *   ACK          → accusé de réception (hôte → client)
 *   BOARD:i,x,y,h,size → placement d'un bateau (bidirectionnel)
 *   SHOOT:x,y    → tir (joueur dont c'est le tour → adversaire)
 *   RESULT:r,x,y → résultat du tir (r=0 raté, 1 touché, 2 coulé)
 *   SUNK:i       → bateau i coulé (information complémentaire)
 *   END:winner   → fin de partie (1 = hôte gagne, 2 = client gagne)
 *   QUIT         → abandon
 *
 * ─────────────────────────────────────────────────────────────── */

/* Taille maximale d'un message réseau */
#define NET_BUF 256

/* ─── API publique ──────────────────────────────────────────── */

int  net_host_start(Jeu *jeu);
int  net_client_connect(Jeu *jeu);
int  net_send(int fd, const char *type, const char *payload);
int  net_recv(int fd, char *buf, int bufsize);
void net_close(Jeu *jeu);

void net_send_placement(Jeu *jeu, int boatIndex);
void net_recv_placements(Jeu *jeu);

void net_send_shoot(Jeu *jeu, int x, int y);
int  net_recv_shoot(Jeu *jeu, int *outX, int *outY);

void net_send_result(Jeu *jeu, int result, int x, int y, int sunkIdx);
int  net_recv_result(Jeu *jeu, int *outX, int *outY, int *outSunk);

void net_send_end(Jeu *jeu, int winner);
void net_send_quit(Jeu *jeu);

#endif /* SERVER_H */
