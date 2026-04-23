#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h> // pour srand et rand


//déclaration des fichiers 
#include "createGame.h"
#include "display.h"
#include "test.h"
#include "game.h"


// Codes ANSI pour les couleurs et le style
#define RED   "\033[31m"
#define RESET   "\033[0m"
#define GREEN   "\033[42m"
#define BG_BLACK "\033[40m"
#define BOLD    "\033[1m"
#define BLUE    "\033[34m"
#define BLINK   "\033[5m"


#define RED_BG "\033[41m"
#define GREEN_BG "\033[42m"
#define BLUE_BG "\033[44m"


#define ROWS 10
#define COLS 10



void getTerminalSize(int *rows, int *cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row *3;
    *cols = w.ws_col *3;
}


// Configure le terminal pour lire les touches sans attendre Entrée
void configureTerminal(struct termios *oldSettings) {
    struct termios newSettings;
    tcgetattr(STDIN_FILENO, oldSettings);
    newSettings = *oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
}

// Restaure les paramètres du terminal
void restoreTerminal(struct termios *oldSettings) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldSettings);
}







bool canPlaceBoatAt(Jeu *jeu, int x, int y, int taille, bool horizontal) {
    for (int t = 0; t < taille; t++) {
        int bx = horizontal ? x + t : x;
        int by = horizontal ? y : y + t;

        if (bx < 0 || bx >= COLS || by < 0 || by >= ROWS)
            return false;

        if (strcmp(jeu->enemyGrid[by][bx], "🚢") == 0)
            return false;
    }
    return true;
}

/*void generateOpponentBoard(Jeu *jeu) {
    srand(time(NULL));

    // Initialiser grille ennemie
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            jeu->enemyGrid[i][j] = "🌊";

    for (int i = 0; i < 5; i++) {
        int taille = jeu->boats[i].size;
        bool placed = false;

        int safety = 0; // évite boucle infinie

        while (!placed && safety < 1000) {
            safety++;

            int x = rand() % COLS;
            int y = rand() % ROWS;
            bool horizontal = rand() % 2;

            if (canPlaceBoatAt(jeu, x, y, taille, horizontal)) {

                jeu->boats[i].x = x;
                jeu->boats[i].y = y;
                jeu->boats[i].horizontal = horizontal;

                for (int t = 0; t < taille; t++) {
                    int bx = horizontal ? x + t : x;
                    int by = horizontal ? y : y + t;

                    jeu->enemyGrid[by][bx] = "🚢";
                }

                placed = true;
            }
        }

        if (!placed) {
            printf("Erreur placement bateau %d\n", i);
        }
    }
}*/

void generateOpponentBoard(Jeu *jeu) {
    srand(time(NULL));

    // init grille ennemie
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            jeu->enemyGrid[i][j] = "🌊";

    int tailles[5] = {5, 4, 3, 3, 2};

    for (int i = 0; i < 5; i++) {
        bool placed = false;
        int safety = 0;

        while (!placed && safety < 1000) {
            safety++;

            int x = rand() % COLS;
            int y = rand() % ROWS;
            bool horizontal = rand() % 2;

            // ⚠️ IMPORTANT → vérifier sur enemyGrid
            bool valid = true;

            for (int t = 0; t < tailles[i]; t++) {
                int bx = horizontal ? x + t : x;
                int by = horizontal ? y : y + t;

                if (bx >= COLS || by >= ROWS) {
                    valid = false;
                    break;
                }

                if (strcmp(jeu->enemyGrid[by][bx], "🚢") == 0) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                // 🔥 stocker dans enemyBoats
                Boat *b = &jeu->enemyBoats[i];
                b->x = x;
                b->y = y;
                b->size = tailles[i];
                b->horizontal = horizontal;
                b->placed = true;

                // placer sur la grille
                for (int t = 0; t < tailles[i]; t++) {
                    int bx = horizontal ? x + t : x;
                    int by = horizontal ? y : y + t;
                    jeu->enemyGrid[by][bx] = "🚢";
                }

                placed = true;
            }
        }

        if (!placed) {
            printf("Erreur placement bateau ennemi %d\n", i);
        }
    }
}








int getBoatSize(int selected) {
    switch(selected) {
        case 0: return 5; // porte-avion
        case 1: return 4; // croiseur
        case 2: return 3; // contre-torpilleur
        case 3: return 3;
        case 4: return 2;
        default: return 2;
    }
}

void resetVar(Jeu *jeu) {
    jeu->x = 0;
    jeu->y = 0;
    jeu->taille = 5;
    jeu->horizontal = true;
    jeu->lost = false;
    jeu->tour = 1;
    jeu->end = 0;

    for (int i=0; i<5;i++){
        jeu->boats[i].drowned = false;
        jeu->enemyBoats[i].drowned = false; 
        for(int j=0; j<5;j++){
            jeu->boats[i].touch[j] = 0;
            jeu->boats[i].x = 0;
            jeu->boats[i].y = 0;
            jeu->enemyBoats[i].x = 0;
            jeu->enemyBoats[i].y = 0;
            jeu->enemyBoats[i].touch[j] = 0;
        }
    }

    for(int i=0; i<10;i++){
        for(int j=0 ; j<10;j++){
            jeu->grille[i][j] = "🌊";
            jeu->attackPlayer[i][j] = "    ";
            jeu->enemyGrid[i][j] = "🌊";
            jeu->attackEnnemy[i][j] = "    ";
        }
    }


}


void createGame(Jeu *jeu){
    int remainingBoat = 5;
    int test;
    int RPorteA = 1; int RCroiseur=1; int RContreT=1; int RSousM=1; int RToprilleur = 1;
    
    int plateau = 1;


    //RÉINITIALISATION DES VATIABLES
    
    resetVar(jeu);


    for(int i=0; i<=9;i++){
        for(int b=0 ; b<=9;b++){
            jeu->grille[i][b] = "🌊";
       
        }
    }
    clearScreen();


        const char *options[] = {
            "Porte-Avion",
            "Croiseur",
            "Contre-torpilleur",
            "Sous-marin",
            "Torpilleur"
        };
        int size = sizeof(options) / sizeof(options[0]);

    int choixPosBateau = 0;


    

        
    
        printf("sélectionnez la position de vos bateaux (Col/Lin).\n");
        //printf("Bateaux Restants : %i Croiseur, %i Contre-torpilleur, %i Sous-marin, %i Torpilleur\n",RPorteA,RCroiseur,RContreT,RSousM,RToprilleur);
        //printf(GREEN BOLD);
        //printf("%i Porte-avion",RPorteA);
        printf("%i             %i          %i           %i          %i         ",RPorteA, RCroiseur, RContreT, RSousM, RToprilleur);
        jeu->isplacement = true;
        interactivePlacement(options, size, jeu);
        //choixPosBateau = interactiveMenu(options, size, plateau, jeu);
        jeu->isplacement = false;
        clearScreen();
        afficherPlateau(jeu);

        printf("Récapitulatif\n");
        for(int i=0; i<5;i++) {
            printf("%s : X :%i Y :%i\n",jeu->boats[i].name,jeu->boats[i].x, jeu->boats[i].y);
        }
        printf("Êtes-vous satisfait du plactement ? 1 : oui, 2 : non \n");

        int erreur = 1;
        int choice = 0;
        while (erreur)
            {
                printf(">");
                if (scanf("%i", &choice) != 1)
                {
                    printf("\a");//son
                    printf(RED BOLD);
                    printf("Erreur CRITIQUE  💣 \n");
                    printf("Les caractères autres que 0-9 sont INTERDITS 💣💣💣💣💣💣.\n");
                    choice = 0;
                    printf(RESET);
                    

                    // vider le buffer
                    while (getchar() != '\n');

                    continue;
                }

                if (choice >= 1 && choice <= 2)
                {
                    erreur = 0;

                    switch(choice)
                    {
                        case 1:
                            printf("Placement du plateau adverse...\n");

                            
                            generateOpponentBoard(jeu);
                            if (jeu->isDebug == true){
                                printf("Récapitulatif des bateaux adverse\n");
                                for(int i=0; i<5;i++) {
                                    printf("%s : X :%i Y :%i\n",jeu->enemyBoats[i].name,jeu->enemyBoats[i].x, jeu->enemyBoats[i].y);
                                }
                            }
                            debug(jeu);
                            getchar();
                            
                            playGame(jeu);
                            erreur = 0;
                            break;

                        case 2:
                            createGame(jeu);
                            break;
                        default:
                            playGame(jeu);
                            break;
                    }
                }
                else
                {
                    printf("Erreur : choix invalide\n");
                }
            }


    



    //getchar();
    
    //getchar();
    //scanf("%i",&test);
    
}

