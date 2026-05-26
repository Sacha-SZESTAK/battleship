#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_ttf.h>



//délcaration des fichiers 
#include "../engine/createGame.h"
//#include "display.h"
#include "sdl_button.h"
#include "../engine/test.h"
#include "../engine/game.h"
#include "display.h"




#define ROWS 10
#define COLS 10

//#define WINDOW_W 1024
//#define WINDOW_H 768

#define WINDOW_W 1280
#define WINDOW_H 720









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


int main(){
    fflush(stdout);
    int rows = 30;  // nombre de lignes
    int cols = 130; // nombre de colonnes

    // séquence ANSI pour définir la taille du terminal
    printf("\033[8;%d;%dt", rows, cols);

    printf("Terminal resized to %dx%d\n", rows, cols);
    
    srand(time(NULL));
    int randomVar = rand() % 8; // 0, 1 ou 2 pour choisir le texte à afficher
    

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
    jeu.isGodMode = false;

    strcpy(jeu.version, "1.0.0");
    resetVar(&jeu);

     initGrids(&jeu);
    for (int i=0; i<5;i++){
        jeu.boats[i].drowned = false;
        jeu.enemyBoats[i].drowned = false; 
        for(int j=0; j<5;j++){
            jeu.boats[i].touch[j] = 0;
            jeu.enemyBoats[i].touch[j] = 0;
        }
    }


     /* ── Init SDL ────────────────────────────────────────────────────── */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    /*SDL_Window   *window   = SDL_CreateWindow("Bataille Navale — SDL",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    */
    SDL_Window *window = SDL_CreateWindow(
        "Bataille Navale — SDL",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, 
        WINDOW_H, 
        SDL_WINDOW_SHOWN 
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /* ── Police ──────────────────────────────────────────────────────── */
    /* Remplace le chemin par une police présente sur ton système        */
    TTF_Font *font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 20);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Texture *logo = load_texture("assets/logo.png", renderer);





    /* ── Création des boutons ─────────────────────────────────────────
     *
     *  button_create(btn, x, y, w, h, label, font,
     *                couleur_normale, couleur_survol, couleur_clic,
     *                couleur_texte, renderer)
     *
     * ────────────────────────────────────────────────────────────────── */
    
    Button btn_jouer, btn_quitter, btn_rules, btn_options, btn_retour, btn_informations;

    button_create(&btn_jouer,   540, 295, 200, 55, "Nouvelle partie", font,
                  BTN_COLOR_NAVY, BTN_COLOR_OCEAN, BTN_COLOR_WAVE,
                  BTN_COLOR_WHITE, renderer);

    button_create(&btn_rules, 540, 375, 200, 55, "Règles", font,
                  BTN_COLOR_GREEN,
                  (SDL_Color){50, 180, 100, 255},
                  (SDL_Color){20, 100, 60,  255},
                  BTN_COLOR_WHITE, renderer);

    button_create(&btn_options, 540, 455, 200, 55, "Options", font,
                  BTN_COLOR_RED,
                  (SDL_Color){50, 180, 100, 255},
                  (SDL_Color){20, 100, 60,  255},
                  BTN_COLOR_WHITE, renderer);

    button_create(&btn_informations, 540, 535, 200, 55, "Credits", font,
                  BTN_COLOR_GREEN,
                  (SDL_Color){50, 180, 100, 255},
                  (SDL_Color){20, 100, 60,  255},
                  BTN_COLOR_WHITE, renderer);

    button_create(&btn_quitter, 540, 615, 200, 55, "Quitter", font,
                  BTN_COLOR_RED,
                  (SDL_Color){220, 60, 60, 255},
                  (SDL_Color){130, 20, 20, 255},
                  BTN_COLOR_WHITE, renderer);



    /* Options supplémentaires (optionnel) */
    btn_jouer.border_width = 2;
    btn_jouer.border_color = BTN_COLOR_WAVE;

    btn_options.border_radius = 0;  /* bouton rectangulaire */
    btn_options.border_width = 2;
    btn_options.border_color = BTN_COLOR_LGRAY;

    /* Exemple : désactiver "Rejouer" au démarrage */
    //button_set_enabled(&btn_rejouer, 0);
    

    /* ── Boucle principale ───────────────────────────────────────────── */
    int running = 1;
    SDL_Event event;

    while (running) {



        /* -- Événements -- */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            
               
            /* ---------------------------------------- */
            /* Passer l'événement à chaque bouton */
            button_handle_event(&btn_jouer,   &event);
            button_handle_event(&btn_rules, &event);
            button_handle_event(&btn_quitter, &event);
            button_handle_event(&btn_options, &event);
            button_handle_event(&btn_informations, &event);
        }

        /* -- Logique -- */
        if (button_was_clicked(&btn_jouer)) {
            printf(">>> Nouvelle partie lancée !\n");
            createGame(renderer, logo, WINDOW_W, WINDOW_H, &jeu, font);
            //button_set_enabled(&btn_rejouer, 1);   /* on peut rejouer maintenant */

            //jouer(&btn_retour, font, renderer);
        }
        if (button_was_clicked(&btn_rules)) {
            printf(">>> Règles !\n");
            rules(&btn_retour, font, renderer);
            //button_set_enabled(&btn_test, 1);   
        }
        if (button_was_clicked(&btn_quitter)) {
            printf(">>> Au revoir !\n");
            running = 0;
        }
        if (button_was_clicked(&btn_options)) {
            printf(">>> Options !\n");
        }
        if (button_was_clicked(&btn_informations)) {
            printf(">>> Informations !\n");
        }

        /* 2. LOGIQUE ET CALCULS D'ANIMATION */
        // On déclare et on calcule ICI, juste avant de s'en servir
        float time = SDL_GetTicks() / 900.0f;
        float scale = 1.5f + (sinf(time * 5.0f) * 0.1f); 
        double angle = 20.0; 
        SDL_Color jaune_minecraft = {255, 255, 0, 255};

        /* -- Rendu -- */
        // 1. On efface d'abord !
        SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
        SDL_RenderClear(renderer);

        // 2. On dessine le logo
        displayLogo(renderer, logo, 540 + 450, 10);

        // 3. ON DESSINE LE SPLASH TEXT ICI (après le Clear)
        if (font == NULL) printf("ERREUR : La police n'est pas chargée !\n");
        switch (randomVar) {
            case 0:
                draw_text_ex(renderer, font, "Titanic 2 !", 900, 180, jaune_minecraft, angle, scale);
                break;
            case 1:
                draw_text_ex(renderer, font, "Le jeu de bataille navale ultime !", 800, 220, jaune_minecraft, angle, scale);
                break;
            case 2:
                draw_text_ex(renderer, font, "Unsinkable?", 900, 220, jaune_minecraft, angle, scale);
                break;
            case 3: 
                draw_text_ex(renderer, font, "404 Ship Not Found", 900, 220, jaune_minecraft, angle, scale);
                break;
            case 4:
                draw_text_ex(renderer, font, "Sink Different", 900, 220, jaune_minecraft, angle, scale);
                break;
            case 5:
                draw_text_ex(renderer, font, "Torpille.exe", 800, 220, jaune_minecraft, angle, scale);
                break;
            case 6:
                draw_text_ex(renderer, font, "Control+Sink", 800, 220, jaune_minecraft, angle, scale);
                break;
            
            
        }
        //draw_text_ex(renderer, font, "Titanic 2 !", 900, 180, jaune_minecraft, angle, scale);
        //draw_text_ex(renderer, font, "Le jeu de bataille navale ultime !", 900, 220, jaune_minecraft, angle, scale);
       // draw_text_ex(renderer, font, "Unsinkable?", 900, 220, jaune_minecraft, angle, scale);
        
        // 4. On dessine les boutons
        button_draw(&btn_jouer,   renderer);
        button_draw(&btn_rules, renderer);
        button_draw(&btn_quitter, renderer);
        button_draw(&btn_options, renderer);
        button_draw(&btn_informations, renderer);

        // 5. On affiche enfin le tout
        SDL_RenderPresent(renderer);

       
    }

    /* ── Nettoyage ───────────────────────────────────────────────────── */
    button_destroy(&btn_jouer);
    button_destroy(&btn_rules);
    button_destroy(&btn_quitter);
    button_destroy(&btn_options);
    button_destroy(&btn_informations);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;


    

   




}