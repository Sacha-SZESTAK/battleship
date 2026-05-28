#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "createGame.h"
#include "game.h"
#include "server.h"

#define ROWS 10
#define COLS 10

/* ─── Codes ANSI (locaux) ──────────────────────────────────── */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

/* ═══════════════════════════════════════════════════════════════
 *  Primitives réseau bas niveau
 * ═══════════════════════════════════════════════════════════════ */

/*
 * net_send – envoie "TYPE:payload\n" sur le socket fd.
 * Si payload est NULL ou vide, envoie juste "TYPE\n".
 */
int net_send(int fd, const char *type, const char *payload) {
    char buf[NET_BUF];
    int  len;

    if (payload && payload[0] != '\0')
        len = snprintf(buf, sizeof(buf), "%s:%s\n", type, payload);
    else
        len = snprintf(buf, sizeof(buf), "%s\n", type);

    int sent = write(fd, buf, len);
    return sent;
}

/*
 * net_recv – lit caractère par caractère jusqu'à '\n' ou EOF.
 * Stocke la ligne (sans '\n') dans buf[0..bufsize-1].
 * Retourne la longueur, -1 en cas d'erreur / connexion fermée.
 */
int net_recv(int fd, char *buf, int bufsize) {
    int i = 0;
    char c;

    while (i < bufsize - 1) {
        int r = read(fd, &c, 1);
        if (r <= 0) return -1;   /* déconnexion ou erreur */
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

/* ═══════════════════════════════════════════════════════════════
 *  Connexion
 * ═══════════════════════════════════════════════════════════════ */

/*
 * net_host_start – crée un socket TCP, bind sur jeu->netPort,
 * listen, accepte UN client, puis échange le handshake HELLO/ACK.
 */
int net_host_start(Jeu *jeu) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("socket");
        return -1;
    }

    /* Réutilisation immédiate du port après fermeture */
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)jeu->netPort);

    if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(serverFd);
        return -1;
    }

    if (listen(serverFd, 1) < 0) {
        perror("listen");
        close(serverFd);
        return -1;
    }

    printf(CYAN "En attente d'un joueur sur le port %d...\n" RESET, jeu->netPort);
    printf("(Communiquez votre IP locale à votre adversaire)\n");
    fflush(stdout);

    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int commFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientLen);
    close(serverFd);   /* on n'accepte qu'une connexion */

    if (commFd < 0) {
        perror("accept");
        return -1;
    }

    printf(GREEN "Joueur connecté depuis %s\n" RESET,
           inet_ntoa(clientAddr.sin_addr));

    /* Handshake : on attend HELLO, on répond ACK */
    char buf[NET_BUF];
    if (net_recv(commFd, buf, sizeof(buf)) < 0 || strncmp(buf, "HELLO", 5) != 0) {
        fprintf(stderr, "Handshake échoué (attendu HELLO, reçu : %s)\n", buf);
        close(commFd);
        return -1;
    }
    net_send(commFd, "ACK", "");

    jeu->netFd     = commFd;
    jeu->netStatus = NET_CONNECTED;
    jeu->netRole   = ROLE_HOST;
    return 0;
}

/*
 * net_client_connect – se connecte à jeu->netIp : jeu->netPort,
 * envoie HELLO, attend ACK.
 */
int net_client_connect(Jeu *jeu) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)jeu->netPort);

    if (inet_pton(AF_INET, jeu->netIp, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide : %s\n", jeu->netIp);
        close(fd);
        return -1;
    }

    printf(CYAN "Connexion à %s:%d...\n" RESET, jeu->netIp, jeu->netPort);
    fflush(stdout);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    /* Handshake : on envoie HELLO, on attend ACK */
    net_send(fd, "HELLO", "");

    char buf[NET_BUF];
    if (net_recv(fd, buf, sizeof(buf)) < 0 || strncmp(buf, "ACK", 3) != 0) {
        fprintf(stderr, "Handshake échoué (attendu ACK, reçu : %s)\n", buf);
        close(fd);
        return -1;
    }

    printf(GREEN "Connecté à l'hôte !\n" RESET);

    jeu->netFd     = fd;
    jeu->netStatus = NET_CONNECTED;
    jeu->netRole   = ROLE_CLIENT;
    return 0;
}

/*
 * net_close – ferme le socket et remet le statut à déconnecté.
 */
void net_close(Jeu *jeu) {
    if (jeu->netFd >= 0) {
        close(jeu->netFd);
        jeu->netFd = -1;
    }
    jeu->netStatus = NET_DISCONNECTED;
}

/* ═══════════════════════════════════════════════════════════════
 *  Flux de jeu – placement
 * ═══════════════════════════════════════════════════════════════ */

/*
 * net_send_placement – envoie "BOARD:i,x,y,h,size\n"
 * où h=1 horizontal, h=0 vertical.
 */
void net_send_placement(Jeu *jeu, int boatIndex) {
    Boat *b = &jeu->boats[boatIndex];
    char payload[64];
    snprintf(payload, sizeof(payload), "%d,%d,%d,%d,%d",
             boatIndex, b->x, b->y, b->horizontal ? 1 : 0, b->size);
    net_send(jeu->netFd, "BOARD", payload);
}

/*
 * net_recv_placements – reçoit 5 messages BOARD de l'adversaire.
 * Les enregistre dans jeu->enemyBoats et jeu->enemyGrid.
 */
void net_recv_placements(Jeu *jeu) {
    /* Réinitialiser la grille ennemie */
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            jeu->enemyGrid[i][j] = (Case){CASE_VIDE, -1};

    for (int received = 0; received < 5; ) {
        char buf[NET_BUF];
        if (net_recv(jeu->netFd, buf, sizeof(buf)) < 0) {
            fprintf(stderr, RED "Connexion perdue lors de la réception des placements.\n" RESET);
            jeu->netStatus = NET_ERROR;
            return;
        }

        if (strncmp(buf, "BOARD:", 6) != 0) continue; /* ignorer autres messages */

        int idx, x, y, h, size;
        if (sscanf(buf + 6, "%d,%d,%d,%d,%d", &idx, &x, &y, &h, &size) != 5) continue;
        if (idx < 0 || idx > 4) continue;

        Boat *b    = &jeu->enemyBoats[idx];
        b->x       = x;
        b->y       = y;
        b->horizontal = (h == 1);
        b->size    = size;
        b->placed  = true;
        b->drowned = false;
        for (int k = 0; k < 5; k++) b->touch[k] = 0;

        for (int t = 0; t < size; t++) {
            int bx = b->horizontal ? x + t : x;
            int by = b->horizontal ? y     : y + t;
            if (bx < COLS && by < ROWS)
                jeu->enemyGrid[by][bx] = (Case){CASE_BATEAU, idx};
        }
        received++;
    }
}

/* ═══════════════════════════════════════════════════════════════
 *  Flux de jeu – tirs
 * ═══════════════════════════════════════════════════════════════ */

/*
 * net_send_shoot – envoie "SHOOT:x,y\n".
 */
void net_send_shoot(Jeu *jeu, int x, int y) {
    char payload[32];
    snprintf(payload, sizeof(payload), "%d,%d", x, y);
    net_send(jeu->netFd, "SHOOT", payload);
}

/*
 * net_recv_shoot – reçoit "SHOOT:x,y" de l'adversaire,
 * l'applique sur la grille locale (jeu->grille, jeu->attackEnnemy),
 * calcule le résultat, et renvoie le résultat à l'adversaire via
 * net_send_result. Retourne un ShootResult.
 */
int net_recv_shoot(Jeu *jeu, int *outX, int *outY) {
    char buf[NET_BUF];

    while (1) {
        if (net_recv(jeu->netFd, buf, sizeof(buf)) < 0) {
            jeu->netStatus = NET_ERROR;
            return SHOOT_RATE;
        }
        if (strncmp(buf, "QUIT", 4) == 0) {
            jeu->end = 1;   /* adversaire abandon → on gagne */
            return SHOOT_RATE;
        }
        if (strncmp(buf, "SHOOT:", 6) == 0) break;
    }

    int x, y;
    if (sscanf(buf + 6, "%d,%d", &x, &y) != 2) {
        net_send_result(jeu, SHOOT_RATE, 0, 0, -1);
        return SHOOT_RATE;
    }
    if (outX) *outX = x;
    if (outY) *outY = y;

    /* ── Appliquer le tir sur notre propre grille ── */
    bool hit = false;
    int  sunkIdx = -1;

    /* Cas déjà tiré : on ne devrait pas arriver là, mais protection */
    if (jeu->attackEnnemy[y][x].etat != CASE_VIDE) {
        net_send_result(jeu, SHOOT_RATE, x, y, -1);
        return SHOOT_RATE;
    }

    for (int i = 0; i < 5; i++) {
        Boat *b = &jeu->boats[i];
        if (b->drowned) continue;

        for (int t = 0; t < b->size; t++) {
            int bx = b->horizontal ? b->x + t : b->x;
            int by = b->horizontal ? b->y     : b->y + t;

            if (x == bx && y == by) {
                b->touch[t] = 1;
                jeu->attackEnnemy[y][x] = (Case){CASE_TOUCHE, i};
                jeu->grille[y][x]       = (Case){CASE_TOUCHE, i};
                hit = true;
                break;
            }
        }
        if (hit) break;
    }

    if (!hit) {
        jeu->attackEnnemy[y][x] = (Case){CASE_RATE, -1};
        jeu->grille[y][x]       = (Case){CASE_RATE, -1};
    }

    int result = SHOOT_RATE;
    if (hit) {
        /* Vérifier si le bateau est coulé */
        jeu->tour = 0; /* IA / adversaire réseau vient de tirer */
        sunkIdx = testSunk(jeu);
        result = (sunkIdx >= 0) ? SHOOT_COULE : SHOOT_TOUCHE;
    }

    /* Envoyer le résultat à l'adversaire */
    net_send_result(jeu, result, x, y, sunkIdx);

    /* Mettre à jour j2Replay (l'adversaire rejoue si touché) */
    jeu->j2Replay = hit;

    return result;
}

/*
 * net_send_result – envoie "RESULT:r,x,y\n" puis si bateau coulé
 * "SUNK:i\n".
 */
void net_send_result(Jeu *jeu, int result, int x, int y, int sunkIdx) {
    char payload[32];
    snprintf(payload, sizeof(payload), "%d,%d,%d", result, x, y);
    net_send(jeu->netFd, "RESULT", payload);

    if (sunkIdx >= 0) {
        char si[16];
        snprintf(si, sizeof(si), "%d", sunkIdx);
        net_send(jeu->netFd, "SUNK", si);
    }
}

/*
 * net_recv_result – reçoit "RESULT:r,x,y" (et éventuellement
 * "SUNK:i") pour le tir que l'on vient d'envoyer.
 * Met à jour attackPlayer / enemyGrid.
 * Retourne un ShootResult.
 */
int net_recv_result(Jeu *jeu, int *outX, int *outY, int *outSunk) {
    char buf[NET_BUF];
    int result = SHOOT_RATE;
    int rx = 0, ry = 0;
    if (outSunk) *outSunk = -1;

    /* Lire RESULT */
    while (1) {
        if (net_recv(jeu->netFd, buf, sizeof(buf)) < 0) {
            jeu->netStatus = NET_ERROR;
            return SHOOT_RATE;
        }
        if (strncmp(buf, "QUIT", 4) == 0) {
            jeu->end = 1;
            return SHOOT_RATE;
        }
        if (strncmp(buf, "RESULT:", 7) == 0) break;
    }

    int r;
    if (sscanf(buf + 7, "%d,%d,%d", &r, &rx, &ry) == 3) {
        result = r;
        if (outX) *outX = rx;
        if (outY) *outY = ry;
    }

    /* Mettre à jour le plateau des tirs joueur */
    if (result == SHOOT_TOUCHE || result == SHOOT_COULE) {
        jeu->attackPlayer[ry][rx] = (Case){CASE_TOUCHE, -1};
        jeu->enemyGrid[ry][rx]    = (Case){CASE_TOUCHE, -1};
    } else {
        jeu->attackPlayer[ry][rx] = (Case){CASE_RATE, -1};
    }

    /* Si bateau coulé, lire SUNK */
    if (result == SHOOT_COULE) {
        /* Essayer de lire SUNK dans les prochains octets disponibles */
        /* On utilise un recv non bloquant via MSG_DONTWAIT */
        char buf2[NET_BUF];
        /* On fait un petit peek : on tente de lire avec un timeout implicite */
        /* Méthode simple : on lit le prochain message directement */
        if (net_recv(jeu->netFd, buf2, sizeof(buf2)) > 0 &&
            strncmp(buf2, "SUNK:", 5) == 0) {
            int si = -1;
            sscanf(buf2 + 5, "%d", &si);
            if (outSunk) *outSunk = si;

            /* Marquer le bateau ennemi comme coulé */
            if (si >= 0 && si < 5) {
                jeu->enemyBoats[si].drowned = true;
                /* Marquer toutes ses cases comme COULE dans attackPlayer */
                Boat *b = &jeu->enemyBoats[si];
                for (int t = 0; t < b->size; t++) {
                    int bx = b->horizontal ? b->x + t : b->x;
                    int by = b->horizontal ? b->y     : b->y + t;
                    if (bx < COLS && by < ROWS) {
                        jeu->attackPlayer[by][bx] = (Case){CASE_TOUCHE, si};
                        jeu->enemyGrid[by][bx]    = (Case){CASE_TOUCHE, si};
                    }
                }
            }
        }
    }

    jeu->j1Replay = (result != SHOOT_RATE);
    return result;
}

/*
 * net_send_end – envoie "END:winner\n".
 */
void net_send_end(Jeu *jeu, int winner) {
    char payload[8];
    snprintf(payload, sizeof(payload), "%d", winner);
    net_send(jeu->netFd, "END", payload);
}

/*
 * net_send_quit – envoie "QUIT\n".
 */
void net_send_quit(Jeu *jeu) {
    net_send(jeu->netFd, "QUIT", "");
}
