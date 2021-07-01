#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>

typedef struct {
    bool running;
    bool paused;
    SDL_Window* window;
    SDL_Renderer* renderer;
    int up;
    int down;
} App;

typedef struct {
    SDL_Rect position;
    int speed;
    int score;
} Paddle;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float speed;
} Ball;

#endif
