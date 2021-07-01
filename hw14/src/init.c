#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "init.h"
#include "defs.h"

void Init(char* string) {
    int window_flags, renderer_flags;

    window_flags = 0;
    renderer_flags = SDL_RENDERER_ACCELERATED;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Cannot initialize SDL: %s.", SDL_GetError());
        exit(1);
    }
    
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        fprintf(stderr, "Failed to initialize audio: %s", Mix_GetError());
        exit(1);
    }
    
    if (TTF_Init() == -1) {
        fprintf(stderr, "SDL_ttf could not be initialized: %s", TTF_GetError());
        exit(1);
    }
    
    app.window = SDL_CreateWindow(
        string,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        window_flags
    );

    if (app.window == NULL) {
        fprintf(stderr, "Could not create window: %s", SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    app.renderer = SDL_CreateRenderer(app.window, -1, renderer_flags);

    if (app.renderer == NULL) {
        fprintf(stderr, "Could not create renderer: %s", SDL_GetError());
        exit(1);
    }
}

void CleanUp(void) {
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}
