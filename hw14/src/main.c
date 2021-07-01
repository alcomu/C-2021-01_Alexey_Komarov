#include "main.h"

int main(int argc, char *argv[])
{
    long then;
    float remainder, bounceAngle;
    char buffer[512];

    SDL_Texture* ScorePlayer1;
    SDL_Texture* ScorePlayer2;

    memset(&app, 0, sizeof(App));
    memset(&player1, 0, sizeof(Paddle));
    memset(&player2, 0, sizeof(Paddle));

    Init("ping pong");

    // Setup FPS
    then = SDL_GetTicks();
    remainder = 0;

    player1 = malloc(sizeof(Paddle));
    player2 = malloc(sizeof(Paddle));

    Ball ball;
    ball = BallPosition(ball);

    // Set ball rect properties
    SDL_Rect ball_rect;
    BallProps(&ball_rect, &ball);

    // Set paddle positions
    PlayerPositions(player1, player2);

    // Set speed
    SetSpeed(player1, player2);

    // Set initial scores
    SetScore(player1, player2);

    // Set up sound effect
    Mix_Chunk* pingHit;
    pingHit = malloc(sizeof(Mix_Chunk));
    pingHit = LoadSoundEffect("./sounds/ping_hit.wav");

    // Game loop
    app.running = true;
    app.paused = false;
    while (app.running) {
        PrepareScene();

        GetInput();

        MovePaddle(player1);

        MoveBall(&ball, &ball_rect);

        CollisionCheck(&ball, &ball_rect, player1, player2, pingHit);

        sprintf(buffer, "%d", player1->score);
        ScorePlayer1 = LoadFont("./fonts/FreeSans.ttf", buffer);
        sprintf(buffer, "%d", player2->score);
        ScorePlayer2 = LoadFont("./fonts/FreeSans.ttf", buffer);

        DrawFont(ScorePlayer1, SCREEN_WIDTH/4, 10, 0, 255, 0);
        DrawFont(ScorePlayer2, SCREEN_WIDTH/4 + SCREEN_WIDTH/2, 10, 0, 0, 255);
        BlitRect(app.renderer, &ball_rect, 255, 255, 255);
        BlitRect(app.renderer, &player1->position, 0, 255, 0);
        BlitRect(app.renderer, &player2->position, 0, 0, 255);

        PlayerBounds(player1);

        PresentScene();

        CapFramerate(&then, &remainder);
    }

    CleanUp();

    CleanupAudio(pingHit);

    free(player1);
    free(player2);

    return 0;
}
