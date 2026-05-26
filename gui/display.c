#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>


#include "../engine/createGame.h"
#include "sdl_button.h"
#include "display.h"

void rules(Button *btn_retour, TTF_Font *font, SDL_Renderer *renderer) {
    SDL_Event event;
    int running = 1;

    

    //initialisation du logo
    SDL_Texture *logo = load_texture("assets/logo.png", renderer);
    

    // Initialisation du bouton (on le fait une seule fois au début)
    button_create(btn_retour, 20, 20, 150, 45, "Retour", font,
                    BTN_COLOR_RED,
                    (SDL_Color){200, 50, 50, 255},
                    (SDL_Color){150, 20, 20, 255},
                    BTN_COLOR_WHITE, renderer);

    while (running) {
        /* 1. GESTION DES ÉVÉNEMENTS */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Si on quitte, on peut soit fermer le jeu soit juste sortir des règles
                exit(0); 
            }
            button_handle_event(btn_retour, &event);
        }

        /* 2. LOGIQUE */
        if (button_was_clicked(btn_retour)) {
            printf(">>> Retour au menu !\n");
            running = 0; // On casse la boucle proprement
        }

        
        


        /* 3. RENDU (UN SEUL BLOC) */
        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
        SDL_RenderClear(renderer);

        //displayLogo(renderer, logo, 540+450, 10);
        // Afficher le Titre (centré dynamiquement)
        draw_text(renderer, font, "REGLES DU JEU", (540), 50, (SDL_Color){255, 215, 0, 255});

        // Afficher les lignes de texte
        SDL_Color blanc = {255, 255, 255, 255};
        draw_text(renderer, font, "- Placez vos bateaux strategiquement.", 100, 150, blanc);
        draw_text(renderer, font, "- Coulez tous les navires ennemis.", 100, 190, blanc);
        draw_text(renderer, font, "- Le premier qui n'a plus de flotte a perdu.", 100, 230, blanc);

        // Dessiner le bouton de retour par-dessus
        button_draw(btn_retour, renderer);

        // Envoyer le tout à la carte graphique
        SDL_RenderPresent(renderer);
    }

    // Optionnel : on peut détruire le bouton ici si on veut libérer la texture
    SDL_DestroyTexture(logo);
    // button_destroy(btn_retour); 
}

void displayLogo(SDL_Renderer *renderer, SDL_Texture *logoTexture, int win_w, int win_h) {
    if (!logoTexture) return;

    SDL_Rect dest;
    // On définit la taille du logo (ex: 150x150)
    dest.w = 1983/3;
    dest.h = 793/3;
    
    // On le place par exemple en haut à droite avec une petite marge
    dest.x = win_w - dest.w - 20; 
    dest.y = 20;

    SDL_RenderCopy(renderer, logoTexture, NULL, &dest);
}


void draw_cell(SDL_Renderer *renderer, int x, int y, int w, int h,
               SDL_Color bg, SDL_Color border) {
    // Fond de la cellule
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);

    // Bordure
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &rect);
}

/*void draw_text(SDL_Renderer *renderer, TTF_Font *font,
               const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dst = {x + 5, y + 5, surface->w, surface->h}; // +5 = padding
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}*/

void draw_table(SDL_Renderer *renderer, TTF_Font *font,
                char *data[3][3], int rows, int cols,
                int start_x, int start_y, int cell_w, int cell_h) {

    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color gray   = {200, 200, 200, 255};
    SDL_Color black  = {0,   0,   0,   255};
    SDL_Color header = {70,  130, 180, 255}; // bleu pour l'en-tête

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int x = start_x + col * cell_w;
            int y = start_y + row * cell_h;

            SDL_Color bg = (row == 0) ? header : white;
            SDL_Color fg = (row == 0) ? white : black;

            draw_cell(renderer, x, y, cell_w, cell_h, bg, gray);
            draw_text(renderer, font, data[row][col], x, y, fg);
        }
    }
}

void afficherPlateau(SDL_Renderer *renderer, SDL_Texture *logoTexture,
                     int win_w, int win_h, Jeu *jeu, TTF_Font *font) {
    if (!renderer || !jeu || !font) {
        //fprintf(stderr, "afficherPlateau: paramètre NULL\n");
        return;
    }

    //fprintf(stderr, "DEBUG: entrée afficherPlateau\n");
    //fprintf(stderr, "DEBUG: renderer=%p jeu=%p font=%p\n", renderer, jeu, font);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray  = {150, 150, 150, 255};
    SDL_Color blue  = {70,  130, 180, 255};
    SDL_Color red   = {200, 50,  50,  255};
    SDL_Color miss  = {100, 100, 200, 255};
    SDL_Color sunk  = {80,  0,   0,   255};

    int cell_w = 50, cell_h = 50;
    int start_x = 50, start_y = 50;

    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            //fprintf(stderr, "DEBUG: case [%d][%d] etat=%d\n", row, col, jeu->grille[row][col].etat);

            int x = start_x + col * cell_w;
            int y = start_y + row * cell_h;

            Case c = jeu->grille[row][col];
            SDL_Color bg;
            const char *texte = NULL;

            switch (c.etat) {
                case CASE_TOUCHE:  bg = red;  texte = "X"; break;
                case CASE_RATE:    bg = miss; texte = "O"; break;
                case CASE_COULE:   bg = sunk; texte = "#"; break;
                case CASE_BATEAU:  bg = (SDL_Color){50, 180, 50, 255}; texte = "B"; break;
                case CASE_VIDE:
                default:           bg = blue; break;
            }

            //fprintf(stderr, "DEBUG: draw_cell [%d][%d]\n", row, col);
            draw_cell(renderer, x, y, cell_w, cell_h, bg, gray);

            if (texte != NULL) {
                //fprintf(stderr, "DEBUG: draw_text [%d][%d] texte=%s\n", row, col, texte);
                draw_text(renderer, font, texte, x + 15, y + 12, white);
            }
        }
    }
    //fprintf(stderr, "DEBUG: fin afficherPlateau\n");
}
    
void createGame(SDL_Renderer *renderer, SDL_Texture *logoTexture,
                int win_w, int win_h, Jeu *jeu, TTF_Font *font) {
    SDL_Event event;
    int running = 1;

    while (running) {
        /* 1. ÉVÉNEMENTS */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0; // retour au menu
            }
        }
        draw_text(renderer, font, "Placement des bateaux", 5, 10, (SDL_Color){255, 255, 255, 255});

        /* 2. RENDU */
        
        SDL_RenderClear(renderer);

        afficherPlateau(renderer, logoTexture, win_w, win_h, jeu, font);

        SDL_RenderPresent(renderer);
        SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    }
}

