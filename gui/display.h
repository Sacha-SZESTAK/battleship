#ifndef GUI_DISPLAY_H
#define GUI_DISPLAY_H

#include <SDL.h>
#include <SDL_ttf.h>

// On inclut directement le header qui définit le type Button
#include "sdl_button.h" 

// On inclut le moteur
#include "../engine/createGame.h"

// Le prototype est maintenant correct car Button est connu via l'include au-dessus
void rules(Button *btn_retour, TTF_Font *font, SDL_Renderer *renderer);

void displayLogo(SDL_Renderer *renderer, SDL_Texture *logo, int win_w, int win_h);

void afficherPlateau(SDL_Renderer *renderer, SDL_Texture *logoTexture, int win_w, int win_h, Jeu *jeu, TTF_Font *font);
void createGame(SDL_Renderer *renderer, SDL_Texture *logoTexture,int win_w, int win_h, Jeu *jeu, TTF_Font *font);

#endif