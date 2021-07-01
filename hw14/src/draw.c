#include "draw.h"

void PrepareScene(void) {
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);
}


void PresentScene(void) {
    SDL_RenderPresent(app.renderer);
}

SDL_Texture* LoadFont(char* filename, char buffer[]) {
    TTF_Font* font;
    SDL_Color color = {255, 255, 255};

    font = TTF_OpenFont(filename, 24);
    if (!font) {
        fprintf(stderr, "Could not open font: %s.", TTF_GetError());
    }

    SDL_Surface* surfaceFont;
    surfaceFont = TTF_RenderText_Solid(font, buffer, color);
    if (!surfaceFont) {
        fprintf(stderr, "Surface could not be initialized: %s.", SDL_GetError());
    }

    SDL_Texture* message;
    message = SDL_CreateTextureFromSurface(app.renderer, surfaceFont);
    if (!message) {
        fprintf(stderr, "Texture could not be initialized: %s.", SDL_GetError());
    }

    SDL_FreeSurface(surfaceFont);
    TTF_CloseFont(font);

    return message;
}

void DrawFont(SDL_Texture* fontTexture, int x, int y, int c1, int c2, int c3) {
    SDL_Rect message_rect;

    message_rect.x = x;
    message_rect.y = y;
    SDL_QueryTexture(fontTexture, NULL, NULL, &message_rect.w, &message_rect.h);
    SDL_SetTextureColorMod(fontTexture, c1, c2, c3);
    SDL_RenderCopy(app.renderer, fontTexture, NULL, &message_rect);
}

void BlitRect(SDL_Renderer* renderer, const SDL_Rect* rect, int c1, int c2, int c3) {
        SDL_SetRenderDrawColor(renderer, c1, c2, c3, 255);
        SDL_RenderFillRect(renderer, rect);
        SDL_RenderDrawRect(renderer, rect);
}
