#ifndef SDL_BUTTON_H
#define SDL_BUTTON_H

//#include <SDL2/SDL.h>
//#include <SDL2/SDL_ttf.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

/* =========================================================
 *  SDL_BUTTON  —  Bouton réutilisable pour projets SDL2
 * =========================================================
 *
 *  Utilisation minimale :
 *
 *    Button btn;
 *    button_create(&btn, 100, 200, 160, 50, "Jouer", font,
 *                  COLOR_NORMAL, COLOR_HOVER, COLOR_PRESSED,
 *                  COLOR_TEXT);
 *
 *    // Dans la boucle d'événements :
 *    button_handle_event(&btn, &event);
 *
 *    // Dans la boucle de rendu :
 *    button_draw(&btn, renderer);
 *
 *    // Tester le clic :
 *    if (button_was_clicked(&btn)) { ... }
 *
 *    // Libérer à la fin :
 *    button_destroy(&btn);
 * ========================================================= */

/* ---------- Couleurs prêtes à l'emploi ------------------- */
#define BTN_COLOR_NAVY     (SDL_Color){  20,  40,  80, 255}
#define BTN_COLOR_OCEAN    (SDL_Color){  30,  90, 160, 255}
#define BTN_COLOR_WAVE     (SDL_Color){  50, 140, 200, 255}
#define BTN_COLOR_WHITE    (SDL_Color){255, 255, 255, 255}
#define BTN_COLOR_DARK     (SDL_Color){ 20,  20,  20, 255}
#define BTN_COLOR_RED      (SDL_Color){180,  30,  30, 255}
#define BTN_COLOR_GREEN    (SDL_Color){ 30, 140,  80, 255}
#define BTN_COLOR_GRAY     (SDL_Color){ 80,  80,  80, 255}
#define BTN_COLOR_LGRAY    (SDL_Color){200, 200, 200, 255}

/* ---------- États internes ------------------------------- */
typedef enum {
    BTN_STATE_NORMAL  = 0,
    BTN_STATE_HOVER   = 1,
    BTN_STATE_PRESSED = 2,
    BTN_STATE_DISABLED = 3
} ButtonState;

/* ---------- Structure principale ------------------------- */
typedef struct {
    SDL_Rect    rect;           /* position + dimensions          */
    const char *label;          /* texte affiché                  */
    TTF_Font   *font;           /* police (non libérée ici)       */

    SDL_Color   color_normal;   /* couleur fond normal            */
    SDL_Color   color_hover;    /* couleur fond survol            */
    SDL_Color   color_pressed;  /* couleur fond clic              */
    SDL_Color   color_text;     /* couleur du texte               */

    ButtonState state;          /* état courant                   */
    int         clicked;        /* 1 si cliqué ce frame           */
    int         enabled;        /* 0 = désactivé (grisé)          */

    int         border_radius;  /* arrondi des coins (px)         */
    int         border_width;   /* épaisseur bordure (0 = aucune) */
    SDL_Color   border_color;   /* couleur bordure                */

    SDL_Texture *text_texture;  /* cache texture du texte         */
    SDL_Rect     text_rect;     /* position du texte centré       */
} Button;

/* ---------- API publique --------------------------------- */

/**
 * Crée un bouton.
 * @param btn           pointeur vers la structure à initialiser
 * @param x, y          position haut-gauche
 * @param w, h          largeur × hauteur
 * @param label         texte (doit rester valide pendant la vie du bouton)
 * @param font          police TTF (doit être chargée avant)
 * @param normal        couleur de fond par défaut
 * @param hover         couleur de fond au survol
 * @param pressed       couleur de fond au clic
 * @param text_color    couleur du texte
 * @param renderer      renderer SDL pour précalculer la texture texte
 * @return 0 si OK, -1 en cas d'erreur
 */
int button_create(Button *btn,
                  int x, int y, int w, int h,
                  const char *label,
                  TTF_Font *font,
                  SDL_Color normal,
                  SDL_Color hover,
                  SDL_Color pressed,
                  SDL_Color text_color,
                  SDL_Renderer *renderer);

/**
 * Met à jour l'état du bouton selon un événement SDL.
 * À appeler dans la boucle d'événements (SDL_PollEvent).
 */
void button_handle_event(Button *btn, const SDL_Event *event);

/**
 * Dessine le bouton sur le renderer.
 * À appeler dans la boucle de rendu.
 */
void button_draw(Button *btn, SDL_Renderer *renderer);

/**
 * Retourne 1 si le bouton a été cliqué depuis le dernier appel,
 * puis remet le flag à 0.  À appeler UNE FOIS par frame.
 */
int button_was_clicked(Button *btn);

/**
 * Active ou désactive le bouton (grisé si disabled).
 */
void button_set_enabled(Button *btn, int enabled);

/**
 * Change le label et reconstruit la texture texte.
 */
void button_set_label(Button *btn, const char *label, SDL_Renderer *renderer);

/**
 * Libère la texture texte (pas la police, pas le renderer).
 */
void button_destroy(Button *btn);

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

void draw_text_ex(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, double angle, float scale);
   

SDL_Texture* load_texture(const char* filename, SDL_Renderer* renderer);

#endif /* SDL_BUTTON_H */
