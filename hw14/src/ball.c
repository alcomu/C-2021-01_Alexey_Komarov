#include <time.h>
#include "structs.h"
#include "defs.h"
#include "ball.h"

Ball BallPosition(Ball ball) {
    srand(time(NULL));
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.speed = STARTBALLSPEED;
    ball.vx = (rand() % 2 == 0)? 1 * ball.speed : -1 * 1 * ball.speed;
    ball.vy = -1.0f;
    return ball;
}

void BallProps(SDL_Rect* ball_rect, Ball* ball) {
    ball_rect->w = 10;
    ball_rect->h = 10;
    ball_rect->x = ball->x;
    ball_rect->y = ball->y;
}