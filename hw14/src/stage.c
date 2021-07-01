#include "common.h"

void PlayerPositions(Paddle* player1, Paddle* player2) {
    player1->position.x = 20;
    player1->position.y = SCREEN_HEIGHT/2 - 80;
    player1->position.h = 150;
    player1->position.w = 20;

    player2->position.x = SCREEN_WIDTH - 40;
    player2->position.y = SCREEN_HEIGHT/2 - 80;
    player2->position.h = 150;
    player2->position.w = 20;

}

void PlayerBounds(Paddle* player) {
    if (player != NULL) {
        if (player->position.y < 15) {
            player->position.y = 0;
        }

        if (player->position.y > SCREEN_HEIGHT - player->position.h) {
            player->position.y = SCREEN_HEIGHT - player->position.h;
        }
    }
}