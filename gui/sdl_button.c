#include "sdl_button.h"
//#include <string.h>
//#include <stdio.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

/* ── helper : dessine un rectangle avec coins arrondis ──────────────────── */
static void draw_rounded_rect(SDL_Renderer *r, SDL_Rect rect, int radius, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);

    if (radius <= 0) {
        SDL_RenderFillRect(r, &rect);
        return;
    }

    /* corps central + bandes horizontale/verticale */
    SDL_Rect body = {rect.x + radius, rect.y, rect.w - 2 * radius, rect.h};
    SDL_RenderFillRect(r, &body);
    SDL_Rect left  = {rect.x, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_Rect right = {rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_RenderFillRect(r, &left);
    SDL_RenderFillRect(r, &right);

    /* coins : cercles au quart — on les approche avec des pixels */
    int cx[4] = {rect.x + radius,
                 rect.x + rect.w - radius - 1,
                 rect.x + radius,
                 rect.x + rect.w - radius - 1};
    int cy[4] = {rect.y + radius,
                 rect.y + radius,
                 rect.y + rect.h - radius - 1,
                 rect.y + rect.h - radius - 1};

    for (int c4 = 0; c4 < 4; c4++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy <= radius * radius)
                    SDL_RenderDrawPoint(r, cx[c4] + dx, cy[c4] + dy);
            }
        }
    }
}

/* ── helper : contour arrondi ───────────────────────────────────────────── */
static void draw_rounded_rect_outline(SDL_Renderer *r, SDL_Rect rect,
                                      int radius, int thickness, SDL_Color c)
{
    for (int i = 0; i < thickness; i++) {
        SDL_Rect inner = {rect.x + i, rect.y + i,
                          rect.w - 2 * i, rect.h - 2 * i};
        /* version simplifiée : rectangle non arrondi pour la bordure */
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
        SDL_RenderDrawRect(r, &inner);
    }
    (void)radius; /* TODO : arrondir la bordure si besoin */
}

/* ── helper : (re)construire la texture du texte ───────────────────────── */
static int rebuild_text_texture(Button *btn, SDL_Renderer *renderer)
{
    if (btn->text_texture) {
        SDL_DestroyTexture(btn->text_texture);
        btn->text_texture = NULL;
    }
    if (!btn->font || !btn->label || btn->label[0] == '\0')
        return 0;

    SDL_Surface *surf = TTF_RenderUTF8_Blended(btn->font, btn->label, btn->color_text);
    if (!surf) {
        fprintf(stderr, "TTF_RenderUTF8_Blended: %s\n", TTF_GetError());
        return -1;
    }
    btn->text_texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!btn->text_texture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        SDL_FreeSurface(surf);
        return -1;
    }
    /* centrer le texte dans le bouton */
    btn->text_rect.w = surf->w;
    btn->text_rect.h = surf->h;
    btn->text_rect.x = btn->rect.x + (btn->rect.w - surf->w) / 2;
    btn->text_rect.y = btn->rect.y + (btn->rect.h - surf->h) / 2;
    SDL_FreeSurface(surf);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  API publique
 * ═══════════════════════════════════════════════════════════════════════════ */

int button_create(Button *btn,
                  int x, int y, int w, int h,
                  const char *label,
                  TTF_Font *font,
                  SDL_Color normal,
                  SDL_Color hover,
                  SDL_Color pressed,
                  SDL_Color text_color,
                  SDL_Renderer *renderer)
{
    if (!btn) return -1;
    memset(btn, 0, sizeof(*btn));

    btn->rect          = (SDL_Rect){x, y, w, h};
    btn->label         = label;
    btn->font          = font;
    btn->color_normal  = normal;
    btn->color_hover   = hover;
    btn->color_pressed = pressed;
    btn->color_text    = text_color;
    btn->state         = BTN_STATE_NORMAL;
    btn->clicked       = 0;
    btn->enabled       = 1;
    btn->border_radius = 8;
    btn->border_width  = 0;
    btn->border_color  = BTN_COLOR_WHITE;

    return rebuild_text_texture(btn, renderer);
}

/* ── gestion des événements ─────────────────────────────────────────────── */
void button_handle_event(Button *btn, const SDL_Event *event)
{
    if (!btn || !btn->enabled) return;

    btn->clicked = 0;

    switch (event->type) {

    case SDL_MOUSEMOTION: {
        int mx = event->motion.x, my = event->motion.y;
        int inside = SDL_PointInRect(&(SDL_Point){mx, my}, &btn->rect);
        if (btn->state == BTN_STATE_PRESSED) break; /* garde pressed jusqu'au release */
        btn->state = inside ? BTN_STATE_HOVER : BTN_STATE_NORMAL;
        break;
    }

    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT) {
            int mx = event->button.x, my = event->button.y;
            if (SDL_PointInRect(&(SDL_Point){mx, my}, &btn->rect))
                btn->state = BTN_STATE_PRESSED;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT) {
            if (btn->state == BTN_STATE_PRESSED) {
                int mx = event->button.x, my = event->button.y;
                btn->clicked = SDL_PointInRect(&(SDL_Point){mx, my}, &btn->rect);
                btn->state   = btn->clicked ? BTN_STATE_HOVER : BTN_STATE_NORMAL;
            }
        }
        break;

    default:
        break;
    }
}

/* ── rendu ──────────────────────────────────────────────────────────────── */
void button_draw(Button *btn, SDL_Renderer *renderer)
{
    if (!btn) return;

    SDL_Color bg;
    if (!btn->enabled) {
        bg = BTN_COLOR_GRAY;
    } else {
        switch (btn->state) {
        case BTN_STATE_HOVER:   bg = btn->color_hover;   break;
        case BTN_STATE_PRESSED: bg = btn->color_pressed; break;
        default:                bg = btn->color_normal;  break;
        }
    }

    /* décalage visuel quand pressé */
    SDL_Rect draw_rect = btn->rect;
    SDL_Rect text_rect = btn->text_rect;
    if (btn->state == BTN_STATE_PRESSED) {
        draw_rect.y += 2;
        text_rect.y += 2;
    }

    /* fond */
    draw_rounded_rect(renderer, draw_rect, btn->border_radius, bg);

    /* bordure optionnelle */
    if (btn->border_width > 0)
        draw_rounded_rect_outline(renderer, draw_rect,
                                  btn->border_radius, btn->border_width,
                                  btn->border_color);

    /* texte */
    if (btn->text_texture) {
        SDL_Color tc = btn->enabled ? btn->color_text : BTN_COLOR_LGRAY;
        SDL_SetTextureColorMod(btn->text_texture, tc.r, tc.g, tc.b);
        SDL_RenderCopy(renderer, btn->text_texture, NULL, &text_rect);
    }
}

/* ── utilitaires ────────────────────────────────────────────────────────── */

int button_was_clicked(Button *btn)
{
    if (!btn) return 0;
    int c  = btn->clicked;
    btn->clicked = 0;
    return c;
}

void button_set_enabled(Button *btn, int enabled)
{
    if (!btn) return;
    btn->enabled = enabled;
    if (!enabled) btn->state = BTN_STATE_DISABLED;
    else          btn->state = BTN_STATE_NORMAL;
}

void button_set_label(Button *btn, const char *label, SDL_Renderer *renderer)
{
    if (!btn) return;
    btn->label = label;
    rebuild_text_texture(btn, renderer);
}

void button_destroy(Button *btn)
{
    if (!btn) return;
    if (btn->text_texture) {
        SDL_DestroyTexture(btn->text_texture);
        btn->text_texture = NULL;
    }
}


void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    // 1. Sécurité : on vérifie que les entrées sont valides
    if (!renderer || !font || !text || strlen(text) == 0) {
        return;
    }

    // 2. Création de la surface
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        fprintf(stderr, "Erreur RenderText: %s\n", TTF_GetError());
        return;
    }

    // 3. Création de la texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "Erreur Texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface); // Ne pas oublier de libérer la surface ici !
        return;
    }

    // 4. Définition de la zone de rendu
    SDL_Rect rect = { x, y, surface->w, surface->h };
    
    // 5. Affichage
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    // 6. Nettoyage immédiat
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

SDL_Texture* load_texture(const char* filename, SDL_Renderer* renderer) {
    // IMG_LoadTexture s'occupe de créer la surface ET la texture d'un coup
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename);
    
    if (!texture) {
        printf("Erreur : Impossible de charger l'image %s. SDL_image Error: %s\n", filename, IMG_GetError());
    }
    
    return texture;
}

void draw_text_ex(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, double angle, float scale) {
    if (!text || strlen(text) == 0) return;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    // On calcule la taille finale avec l'échelle (scale)
    SDL_Rect rect = { x, y, (int)(surface->w * scale), (int)(surface->h * scale) };
    
    // Le centre de rotation (ici, le milieu du texte)
    SDL_Point center = { rect.w / 2, rect.h / 2 };

    // SDL_RenderCopyEx permet la rotation et le flip
    SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}