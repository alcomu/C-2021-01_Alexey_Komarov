#include "common.h"

void CollisionCheck(Ball* ball, SDL_Rect* ball_rect,
                    Paddle* player1, Paddle* player2,
                    Mix_Chunk* pingHit) {
    float bounce_angle;

    if (ball->y < 0) {
        ball->y = 0;
        ball->vy *= -1;
    }

    if (ball->y + 10 >= SCREEN_HEIGHT) {
        ball->y = SCREEN_HEIGHT - 10;
        ball->vy *= -1;
    }

    if (ball->x + 10 < 0) {
        ball->speed = STARTBALLSPEED;
        ball->x = player1->position.x + player1->position.w;
        ball->y = player1->position.y + player1->position.h/2;
        ball->vx = STARTBALLSPEED;
        player2->score++;
    }

    if (ball->x >= SCREEN_WIDTH) {
        ball->speed = STARTBALLSPEED;
        ball->x = player2->position.x - player2->position.w;
        ball->y = player2->position.y + player2->position.h/2;
        ball->vx = STARTBALLSPEED;
        player1->score++;
    }

    if (SDL_HasIntersection(ball_rect, &player1->position)) {
        Mix_PlayChannel(-1, pingHit, 0);
        ball->speed += 0.5;
        if (ball->speed >= MAXBALLSPEED) {
            ball->speed = MAXBALLSPEED;
        }
        bounce_angle = CalcAngle(player1->position.y, ball->y, player1->position.h);
        ball->vy = sin(bounce_angle) * ball->speed;
        ball->vx = cos(bounce_angle) * ball->speed;
    }

    if (SDL_HasIntersection(ball_rect, &player2->position)) {
        Mix_PlayChannel(-1, pingHit, 0);
        ball->speed += 0.5;
        if (ball->speed >= MAXBALLSPEED) {
            ball->speed = MAXBALLSPEED;
        }
        bounce_angle = CalcAngle(player2->position.y, ball->y, player1->position.h);
        ball->vy = sin(bounce_angle) * ball->speed * -1;
        ball->vx = cos(bounce_angle) * ball->speed * -1;
    }

    if (ball->y < player2->position.y + player2->position.y/2) {
        player2->position.y -= player2->speed;
    }
    if (ball->y > player2->position.y + player2->position.y/2) {
        player2->position.y += player2->speed;
    }
}