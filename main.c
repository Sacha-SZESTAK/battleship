#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>




//délcaration des fichiers 
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
#define WHITE "\033[37m"
#define RED_BG "\033[41m"


#define ROWS 10
#define COLS 10

char version[10] = "1.0.0";



void rules(){
    clearScreen();

    printf(RED "Lire attentivement les règles\n\n" RESET);

    printf(BLUE"1) Regle du jeu\n"RESET);
    printf("La bataille navale est une jeu a 2 joueurs en 1 contre 1,\n");
    printf("ils s'opposent avec comme objectif de detruire la flotte navale adverse, \n");
    printf("vous allez devoir placer 5 bateau (porte-avion, croiseur, contre-torpilleur, sous-marin et torpilleur) en vertical ou horizontal.\n");
    printf("Apres avoir placé vos bateaux vous pourrez démarrer la partie et vous attaquez entre vous avec votre adversaire.\n");
    printf("Pour attaquer votre adversaire vous devrez selectionner une case sur le tableau de jeu si vous touchez vous pouvez continuer,\n");
    printf("et si vous ne touchez pas votre tour est terminer une fois le tour de l'adversaire terminer ça sera de nouveaux votre tour ainsi\n");
    printf("jusqu'à ce que l'un des 2 joueurs est fais couler toute la flote de l'autre.\n\n");

    printf(BLUE"2) Placement sur le tableau\n"RESET);
    printf("Lors de la création d'une partie l'invite vous demande de placer vos bateaux le bateau sélectionné s'affiche en vert ainsi que sur\n");
    printf("le plateau, vous pouvez changer le bateau sélectionné en appuyant sur W ou X.\n");
    printf("Pour placer le bateau sélectionné, utiliser les touches Z Q S D pour le déplacer de gauche à droite et de haut en bas,\n");
    printf("pour l'orienter appuyez sur la touche C.\n");
    printf("une fois que la position vous satisfait appuyez sur entrée pour valider.\n");
    printf("L'invite passera automatiquement au bateau suivant.\n\n");


    printf(BLUE"3) Types de bateaux\n"RESET);
    printf("Il existe 5 types pour 4 tailles de bateaux : \n");
    printf("Porte-avion : 🚢🚢🚢🚢🚢\n");
    printf("Croiseur : 🚢🚢🚢🚢\n");
    printf("Contre-torpilleur : 🚢🚢🚢\n");
    printf("Sous-marin : 🚢🚢🚢\n");
    printf("Torpilleur : 🚢🚢\n");
    

    printf("\n\n\n Appuyez sur une touche pour continuer.\n");
    printf(">");
    getchar();
    getchar();



}





void initGrids(Jeu *jeu) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            jeu->grille[i][j]       = (Case){CASE_VIDE, -1};
            jeu->attackPlayer[i][j] = (Case){CASE_VIDE, -1};
            jeu->attackEnnemy[i][j] = (Case){CASE_VIDE, -1};
            jeu->enemyGrid[i][j]    = (Case){CASE_VIDE, -1};
        }
    }
}

void informations(){
    clearScreen();
    printLogo(2);
    printf("Bataille Navale - Version %s\n\n",version);
    printf("Ce projet a été réalisé par Sacha SZESTAK dans le cadre du module de programmation en C du semestre 2 du projet informatique à UniLaSalle Amiens\n");
    printf("Le projet consiste en la création d'un jeu de bataille navale en ligne de commande, avec une IA basique.\n");
    printf("Le projet a été réalisé en utilisant le langage C, avec une attention particulière portée à la gestion de la mémoire et à l'optimisation du code.\n");
    printf("Le projet a été développé en utilisant la norme C17, et a été testé sur un terminal compatible ANSI.\n");
    printf("Le projet est encore en développement, et de nombreuses fonctionnalités sont prévues pour les futures versions, telles que l'ajout d'un mode multijoueur en ligne, l'amélioration de l'IA, et l'ajout de nouveaux types de bateaux et de cartes.\n");
    printf("\nGithub : https://github.com/Sacha-SZESTAK/battleship\n");
    
    printf("\nCredits : Sacha SZESTAK, Samuel MOATTI - I1 - 2026\n");
    printf("\n\n\n Appuyez sur une touche pour continuer.\n");
    printf(">");
    getchar();
    getchar();


}
int main(){
    fflush(stdout);
    int rows = 30;  // nombre de lignes
    int cols = 130; // nombre de colonnes

    // séquence ANSI pour définir la taille du terminal
    printf("\033[8;%d;%dt", rows, cols);

    printf("Terminal resized to %dx%d\n", rows, cols);
    
    


    Jeu jeu;
    int choix = 1;
    int quitter = 0;
    jeu.x = 0;
    jeu.y = 0;
    jeu.taille = 5;
    jeu.horizontal = true;
    jeu.lost = false;
    jeu.tour = 1;
    jeu.end = 0;
    
    resetVar(&jeu);

    for (int i=0; i<5;i++){
        jeu.boats[i].drowned = false;
        jeu.enemyBoats[i].drowned = false; 
        for(int j=0; j<5;j++){
            jeu.boats[i].touch[j] = 0;
            jeu.enemyBoats[i].touch[j] = 0;
        }
    }


    // Initialisation des noms des bateaux
    strncpy(jeu.boats[0].name, "Porte-Avion", sizeof(jeu.boats[0].name)-1);
    jeu.boats[0].name[sizeof(jeu.boats[0].name)-1] = '\0';

    strncpy(jeu.boats[1].name, "Croiseur", sizeof(jeu.boats[1].name)-1);
    jeu.boats[1].name[sizeof(jeu.boats[1].name)-1] = '\0';

    strncpy(jeu.boats[2].name, "Contre-Torpilleur", sizeof(jeu.boats[2].name)-1);
    jeu.boats[2].name[sizeof(jeu.boats[2].name)-1] = '\0';

    strncpy(jeu.boats[3].name, "Sous-Marin", sizeof(jeu.boats[3].name)-1);
    jeu.boats[3].name[sizeof(jeu.boats[3].name)-1] = '\0';

    strncpy(jeu.boats[4].name, "Torpilleur", sizeof(jeu.boats[4].name)-1);
    jeu.boats[4].name[sizeof(jeu.boats[4].name)-1] = '\0';



    strncpy(jeu.enemyBoats[0].name, "Porte-Avion", sizeof(jeu.enemyBoats[0].name)-1);
    jeu.enemyBoats[0].name[sizeof(jeu.enemyBoats[0].name)-1] = '\0';

    strncpy(jeu.enemyBoats[1].name, "Croiseur", sizeof(jeu.enemyBoats[1].name)-1);
    jeu.enemyBoats[1].name[sizeof(jeu.enemyBoats[1].name)-1] = '\0';

    strncpy(jeu.enemyBoats[2].name, "Contre-Torpilleur", sizeof(jeu.enemyBoats[2].name)-1);
    jeu.enemyBoats[2].name[sizeof(jeu.enemyBoats[2].name)-1] = '\0';

    strncpy(jeu.enemyBoats[3].name, "Sous-Marin", sizeof(jeu.enemyBoats[3].name)-1);
    jeu.enemyBoats[3].name[sizeof(jeu.enemyBoats[3].name)-1] = '\0';

    strncpy(jeu.enemyBoats[4].name, "Torpilleur", sizeof(jeu.enemyBoats[4].name)-1);
    jeu.enemyBoats[4].name[sizeof(jeu.enemyBoats[4].name)-1] = '\0';

    

    initGrids(&jeu);

    while (quitter == 0){
        clearScreen();
        printLogo(1);
        printf("Version : %s\n",version);
        printf("\n\n");
        //printf("\nBATAILLE NAVALE - Version 0.5\n\n");


        printf("Menu Principal\n");
        printf("1) Créer une partie\n");
        printf("2) Règles\n");
        printf("3) Options\n");
        printf("4) Informations\n");
        printf("5) Quitter\n");

        if(jeu.isDebug == true){
            printf(WHITE RED_BG BLINK "\n /!\\ DEBUG ACTIVÉ /!\\ \n"RESET);
        }

        //printf("\n\n\n Copyright Sacha SZESTAK I1 - 2026\n");
        int erreur = 1;
        while (erreur)
            {
                printf(">");
                if (scanf("%i", &choix) != 1)
                {
                    printf("\a");
                    printf(RED BOLD);
                    printf("Erreur CRITIQUE  💣 \n");
                    printf("Les caractères autres que 0-9 sont INTERDITS 💣💣💣💣💣💣.\n");
                    choix = 0;
                    printf(RESET);
                    

                    // vider le buffer
                    while (getchar() != '\n');

                    continue;
                }

                if (choix >= 1 && choix <= 5)
                {
                    erreur = 0;

                    switch(choix)
                    {
                        case 1:
                            //printf("ok");
                            createGame(&jeu);
                            break;

                        case 2:
                            rules();
                            break;
                        case 3:
                            settings(&jeu);
                            break;
                        case 4:
                            informations();
                            break;
                        case 5:
                            quitter = 1;
                            break;
                    }
                }
                else
                {
                    printf("Erreur : choix invalide\n");
                }
            }

        /*}else{
            printf(RED BOLD);
            printf("Erreur critique  💣 \n");
            printf("Les caractères autres que 0-9 sont INTERDITS 💣💣💣💣💣💣.\n");
            choix = 0;
            printf(RESET);
            break;
        }*/
        
    }

}