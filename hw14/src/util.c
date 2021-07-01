#include "common.h"

// Calculate ball angle
float CalcAngle(float paddle_y, float ball_y, float paddle_h) {
   float relative_y = (paddle_y + (paddle_h / 2)) - ball_y;
   float normalized_rel_y = (relative_y / (paddle_h / 2));
   float bounce_angle = normalized_rel_y * MAXANGLE;
   return bounce_angle;
}

// Set speed of paddles
void SetSpeed(Paddle* player1, Paddle* player2) {
    player1->speed = 15;
    player2->speed = 5;
}

// Initialize scores
void SetScore(Paddle* player1, Paddle* player2) {
    player1->score = 0;
    player2->score = 0;
}

void MovePaddle(Paddle* player) {
    if (app.up) {
        player->position.y -= player->speed;
    }
    if (app.down) {
        player->position.y += player->speed;
    }
}

void MoveBall(Ball* ball, SDL_Rect* ball_rect) {
    ball->x += ball->vx;
    ball->y += ball->vy;
    ball_rect->x = (int) ball->x;
    ball_rect->y = (int) ball->y;
}