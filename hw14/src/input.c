#include "input.h"

static void ProcessDownKey(SDL_KeyboardEvent* event);
static void ProcessUpKey(SDL_KeyboardEvent* event);

void GetInput(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                app.running = false;
                exit(0);

            case SDL_KEYDOWN:
                ProcessDownKey(&event.key);
                break;

            case SDL_KEYUP:
                ProcessUpKey(&event.key);
                break;

            default:
                break;
        }
    }
}

static void ProcessDownKey(SDL_KeyboardEvent* event)
{
    if (event->repeat == 0) {
        if (event->keysym.scancode == SDL_SCANCODE_UP) {
            app.up = 1;
        }
        if (event->keysym.scancode == SDL_SCANCODE_DOWN) {
            app.down = 1;
        }
        if (event->keysym.scancode == SDL_SCANCODE_ESCAPE) {
            app.running = false;
        }
    }
}

static void ProcessUpKey(SDL_KeyboardEvent* event) {
    if (event->repeat == 0) {
        if (event->keysym.scancode == SDL_SCANCODE_UP) {
            app.up = 0;
        }
        if (event->keysym.scancode == SDL_SCANCODE_DOWN) {
            app.down = 0;
        }
    }
}
