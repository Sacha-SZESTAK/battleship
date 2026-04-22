#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>


//déclarartion des fichiers
#include "display.h"
#include "game.h"
#include "createGame.h"
#include "test.h"

// Codes ANSI pour les couleurs et le style
#define RED   "\033[31m"
#define RESET   "\033[0m"
#define GREEN   "\033[42m"
#define BG_BLACK "\033[40m"
#define BOLD    "\033[1m"
#define BLUE    "\033[34m"
#define WHITE "\033[37m"

#define BLINK   "\033[5m"


#define RED_BG "\033[41m"
#define GREEN_BG "\033[42m"
#define BLUE_BG "\033[44m"

#define ROWS 10
#define COLS 10



// Efface l'écran
void clearScreen() {
    printf("\033[H\033[J");
}




void printLogo(){
    printf(BLUE BOLD);
    printf("██████╗  █████╗ ████████╗████████╗██╗     ███████╗███████╗██╗  ██╗██╗██████╗ \n");
    printf("██╔══██╗██╔══██╗╚══██╔══╝╚══██╔══╝██║     ██╔════╝██╔════╝██║  ██║██║██╔══██╗\n");
    printf("██████╔╝███████║   ██║      ██║   ██║     █████╗  ███████╗███████║██║██████╔╝\n");
    printf("██╔══██╗██╔══██║   ██║      ██║   ██║     ██╔══╝  ╚════██║██╔══██║██║██╔═══╝ \n");
    printf("██████╔╝██║  ██║   ██║      ██║   ███████╗███████╗███████║██║  ██║██║██║     \n");
    printf("╚═════╝ ╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝╚═╝     \n");
    printf(RESET);
}



void afficherPlateau(Jeu *jeu){

    // Titres
    if (jeu->displayEnnemy){
        printf("╔════════════════════"BLINK WHITE RED_BG"plateau de jeu ENNEMI"RESET"════════════════════╗");
    } else {
        printf("╔════════════════════════plateau de jeu═══════════════════════╗");
    }
    printf("  ");
    if (jeu->displayEnnemy){
    printf("╔══════════════════"BLINK WHITE RED_BG"plateau des tirs ENNEMI"RESET"════════════════════╗\n");
    } else {
        printf("╔══════════════════════plateau des tirs═══════════════════════╗\n");
    }
    // Nombres de colonnes
    printf("║       1    2    3    4    5    6    7    8    9    10       ║");
    printf("  ");
    printf("║       1    2    3    4    5    6    7    8    9    10       ║\n");

    printf("║     ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐     ║");
    printf("  ");
    printf("║     ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐     ║\n");

    for(int i = 0; i < 10; i++)
    {
        // Ligne joueur
        printf("║ %c>  │", 'A' + i);
        for(int j = 0; j < 10; j++)
        {
            bool isBateau = false;

            //plateau de jeu
            // vérifier si la case est occupée par un bateau en placement
            for(int t = 0; t < jeu->taille; t++){
                int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                int by = jeu->horizontal ? jeu->y : jeu->y + t;
                if (bx >= COLS || by >= ROWS) continue;
                if(by == i && bx == j){
                    isBateau = true;
                    break;
                }
            }

            if (strcmp(jeu->grille[i][j], " ➕ ") == 0) {
                printf(RED " ➕ " RESET "│");
            } else if(isBateau && jeu->isplacement){
                if (jeu->num == 1) printf(GREEN_BG BLINK " 🚢 " RESET "│");
                else if (jeu->num == 2) printf(RED_BG BLINK " 🚢 " RESET "│");
                else if (jeu->num == 3) printf(BLUE_BG BLINK " 🚢 " RESET "│");
                else printf(GREEN_BG BLINK " 🚢 " RESET "│");
            //} else if (jeu->isShooting && i == jeu->y && j == jeu->x) {
                //printf(RED BLINK " ➕ " RESET "│");
            } else if(jeu-> displayEnnemy == true){
                printf(" %s │", jeu->enemyGrid[i][j]);
                
            }else {
                printf(" %s │", jeu->grille[i][j]);
            }
        }

        // Espace entre les plateaux
        printf("     ║  ║");

        // plateau des tirs 
        printf(" %c>  │", 'A' + i);
        for(int j = 0; j < 10; j++)
        {
            //if (strcmp(jeu->attackPlayer[i][j], "❌") == 0) {
            //    printf(RED " ❌ " RESET "│"); // tir raté
            //} else if (strcmp(jeu->attackPlayer[i][j], "💥") == 0) {
            //    printf(RED_BG " 💥 " RESET "│"); // tir touché
            //} else
            if (jeu->isShooting && i == jeu->y && j == jeu->x) {
                printf(RED BLINK " ➕ " RESET "│"); // hologramme tir
            }else if(jeu->displayEnnemy == true){
                printf("%s│", jeu->attackEnnemy[i][j]);
            }else{
                printf("%s│", jeu->attackPlayer[i][j]);
            }
            //else {
            //    printf("    │"); // case vide
            //}
        }

        printf("     ║\n");

        if(i < 9){
            printf("║     ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤     ║");
            printf("  ");
            printf("║     ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤     ║\n");
        }
    }

    // Bas des plateaux
    printf("║     └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘     ║");
    printf("  ");
    printf("║     └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘     ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝");
    printf("  ");
    printf("╚═════════════════════════════════════════════════════════════╝\n");
}





/*void afficherPlateau(Jeu *jeu){

    if (jeu->displayEnnemy == true){
        printf("╔════════════════════"RED_BG"plateau de jeu ENNEMI"RESET"════════════════════╗\n");
    }else{
        printf("╔════════════════════════plateau de jeu═══════════════════════╗\n");
    }
    printf("║       1    2    3    4    5    6    7    8    9    10       ║\n");
    printf("║     ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐     ║\n");

    for(int i = 0; i < 10; i++)
    {
        printf("║ %c>  │", 'A' + i);

        for(int j = 0; j < 10; j++)
        {
            
            
                bool isBateau = false;
                
                // vérifier si la case est occupée par le bateau
                for(int t = 0; t < jeu->taille; t++){
                    int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                    int by = jeu->horizontal ? jeu->y : jeu->y + t;


                    //if (bx < 0 || bx >= COLS || by < 0 || by >= ROWS) {
                    //    printf("ERREUR INDEX bx=%d by=%d\n", bx, by);
                    //    continue;
                    //}
                    if (bx >= COLS) continue;
                    if (by >= ROWS) continue;

                    if(by == i && bx == j){
                        isBateau = true;
                        break;
                    }
                }
                if (strcmp(jeu->grille[i][j], " ➕ ") == 0) {
                    printf(RED " ➕ " RESET "│");
                }else if(isBateau && jeu-> isplacement == true){
                    if (jeu->num == 1){
                        printf(GREEN_BG BLINK " 🚢 " RESET"│");
                    }else if (jeu->num == 2){
                        printf(RED_BG BLINK " 🚢 " RESET"│");
                    }else if (jeu->num == 3){
                        printf(BLUE_BG BLINK " 🚢 " RESET"│");
                    }else{
                        printf(GREEN_BG BLINK " 🚢 " RESET"│");
                    }
                } else if (jeu->isShooting == true && i == jeu->y && j == jeu->x) {
                    printf(RED BLINK " ➕ " RESET "│");
                }else {
                    if (jeu->displayEnnemy == false){
                        printf(" %s │", jeu->grille[i][j]);
                    }else{
                        printf(" %s │", jeu->enemyGrid[i][j]);
                    }
                }
                // curseur de tir (hologramme)

            
        }


        printf("     ║\n");

        if(i < 9)
            printf("║     ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤     ║\n");
    }

    printf("║     └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘     ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝\n");
}*/


// Vérifie si un bateau peut être placé à la position actuelle
bool canPlaceBoat(Jeu *jeu) {
    for (int t = 0; t < jeu->taille; t++) {
        int bx = jeu->horizontal ? jeu->x + t : jeu->x;
        int by = jeu->horizontal ? jeu->y : jeu->y + t;

        // vérifier limites
        if (bx >= COLS || by >= ROWS) return false;

        // vérifier si la case est déjà occupée
        if (strcmp(jeu->grille[by][bx], "🚢") == 0) return false;
    }
    return true;
}



void displayHorizontalMenu(const char **options, int size, int selected, int plateau, Jeu *jeu) {
    //clearScreen();  
    if (plateau){
        afficherPlateau(jeu);
    }
        // optionnel : efface l’écran
    for (int i = 0; i < size; i++) {
        if (i == selected) {
            printf(GREEN BOLD " %s " RESET, options[i]); // surligné
        } else {
            printf(" %s ", options[i]);                 // non sélectionné
        }
        printf("  "); // espace entre options
    }
    printf("\n");
    printf("\nAppuyez sur W/X pour naviguer et Entrée pour valider.\n");
}
/*
// Gère la navigation avec les flèches
int interactiveMenu(const char **options, int size, int plateau, Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    char ch;
    displayHorizontalMenu(options, size, selected, plateau, jeu);

    while (1) {
        read(STDIN_FILENO, &ch, 1);

        if (ch == 'w' || ch == 'W') {
            selected = (selected - 1 + size) % size;
            displayHorizontalMenu(options, size, selected, plateau, jeu);
            return selected;
        } else if (ch == 'x' || ch == 'X') {
            selected = (selected + 1) % size;
            displayHorizontalMenu(options, size, selected, plateau, jeu);
            return selected;
        } else if (ch == '\n') { // Entrée
            restoreTerminal(&oldSettings);
            printf("\n"); // passer à la ligne après choix
            return selected;
        }
    }
}*/

/*
int interactivePlacement(const char **options, int size, Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    char ch;

    while (1) {
        clearScreen();

        // Taille du bateau selon sélection
        switch(selected) {
            case 0:
                jeu->taille = 5; 
                jeu->num = 1;
                break; // Porte avion
            case 1:
                jeu->taille = 4; 
                jeu->num = 2;
                break; // Croiseur
            case 2: 
                jeu->taille = 3; 
                jeu->num = 3;
                break; // Contre-torpilleur
            case 3: 
                jeu->taille = 3; 
                jeu->num = 4;
                break; // Sous-marin
            case 4: 
                jeu->taille = 2; 
                jeu->num = 5;
                break; // Torpilleur
            default: 
                jeu->taille = 2; 
                jeu->num = 5;
                break; //défaut
        }

        // Affichage du plateau avec bateau “hologramme”
        afficherPlateau(jeu);

        // Affichage menu horizontal au-dessus ou en-dessous
        displayHorizontalMenu(options, size, selected, 0, jeu);

        // Lecture d’une touche
        read(STDIN_FILENO, &ch, 1);

        // --- Menu bateau ---
        if (ch == 'w' || ch == 'W') selected = (selected - 1 + size) % size;
        else if (ch == 'x' || ch == 'X') selected = (selected + 1) % size;

        // --- Déplacement ---
        else if (ch == 'z' && jeu->y > 0) jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0) jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        // --- Rotation ---
        else if (ch == 'c' || ch == 'C') jeu->horizontal = !jeu->horizontal;

        // --- Placement final ---
        else if (ch == '\n') {
            // Placer le bateau sur la grille
            for (int t = 0; t < jeu->taille; t++) {
                int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                int by = jeu->horizontal ? jeu->y : jeu->y + t;
                jeu->grille[by][bx] = "🚢";
            }
            restoreTerminal(&oldSettings);
            return selected;
        }

        // --- Limiter le bateau à l’intérieur ---
        if (jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if (!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }
}*/


int interactivePlacement(const char **options, int size, Jeu *jeu) {
    struct termios oldSettings;
    configureTerminal(&oldSettings);

    int selected = 0;
    char ch;

    // Tableau pour savoir quels bateaux ont été posés (0 = non posé, 1 = posé)
    int placedBoats[5] = {0, 0, 0, 0, 0};

    while (1) {
        clearScreen();

        // --- Déterminer le bateau sélectionné en sautant ceux déjà posés ---
        while (placedBoats[selected]) {
            selected = (selected + 1) % size;
        }

        // Taille et numéro du bateau
        switch(selected) {
            case 0: jeu->taille = 5; jeu->num = 1; break; // Porte-avion
            case 1: jeu->taille = 4; jeu->num = 2; break; // Croiseur
            case 2: jeu->taille = 3; jeu->num = 3; break; // Contre-torpilleur
            case 3: jeu->taille = 3; jeu->num = 4; break; // Sous-marin
            case 4: jeu->taille = 2; jeu->num = 5; break; // Torpilleur
        }

        // Affichage plateau + menu
        afficherPlateau(jeu);
        displayHorizontalMenu(options, size, selected, 0, jeu);

        // Lecture d'une touche
        read(STDIN_FILENO, &ch, 1);

        // --- Menu bateau ---
        if (ch == 'w' || ch == 'W') {
            int orig = selected;
            do {
                selected = (selected - 1 + size) % size;
            } while (placedBoats[selected] && selected != orig); // saute les bateaux déjà posés
        } else if (ch == 'x' || ch == 'X') {
            int orig = selected;
            do {
                selected = (selected + 1) % size;
            } while (placedBoats[selected] && selected != orig);
        }

        // --- Déplacement ---
        else if (ch == 'z' && jeu->y > 0) jeu->y--;
        else if (ch == 's' && jeu->y < ROWS - 1) jeu->y++;
        else if (ch == 'q' && jeu->x > 0) jeu->x--;
        else if (ch == 'd' && jeu->x < COLS - 1) jeu->x++;

        // --- Rotation ---
        else if (ch == 'c' || ch == 'C') jeu->horizontal = !jeu->horizontal;

        //Ensuite, dans ta boucle de placement, remplace le code qui posait le bateau par :

        // --- Placement final ---
        else if (ch == '\n') {
            if (!canPlaceBoat(jeu)) {
                // On ne peut pas poser ici
                printf("\a"); // bip sonore pour indiquer erreur
                continue;
            }

            int idx = selected; // bateau sélectionné

            Boat *b = &jeu->boats[idx];
            b->x = jeu->x;
            b->y = jeu->y;
            b->size = jeu->taille;
            b->horizontal = jeu->horizontal;
            b->placed = true;

            // Placer le bateau
            for (int t = 0; t < jeu->taille; t++) {
                int bx = jeu->horizontal ? jeu->x + t : jeu->x;
                int by = jeu->horizontal ? jeu->y : jeu->y + t;
                jeu->grille[by][bx] = "🚢";
            }

            // Marquer le bateau comme posé
            placedBoats[selected] = 1;

            // Réinitialiser position et orientation pour le prochain bateau
            jeu->x = 0;
            jeu->y = 0;
            jeu->horizontal = true;

            // Vérifier si tous les bateaux ont été placés
            int allPlaced = 1;
            for (int i = 0; i < size; i++) {
                if (!placedBoats[i]) allPlaced = 0;
            }
            if (allPlaced) break;
        }

        // --- Limites ---
        if (jeu->horizontal && jeu->x + jeu->taille > COLS) jeu->x = COLS - jeu->taille;
        if (!jeu->horizontal && jeu->y + jeu->taille > ROWS) jeu->y = ROWS - jeu->taille;
    }

    restoreTerminal(&oldSettings);
    return 0;
}