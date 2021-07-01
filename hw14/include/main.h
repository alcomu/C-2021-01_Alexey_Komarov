#include "common.h"

extern void CleanUp(void);
extern void Init(char* string);
extern void PrepareScene(void);
extern void PresentScene(void);
//extern void drawTex(SDL_Texture* texture, int x, int y);
//extern SDL_Texture* loadTex(char* filename);
extern Mix_Chunk* LoadSoundEffect(char* filename);
extern void CleanupAudio(Mix_Chunk* soundEffect);
extern void PlayerPositions(Paddle* player1, Paddle* player2);
extern void PlayerBounds(Paddle* player);
extern void CapFramerate(long* then, float* remainder);
extern float CalcAngle(float, float, float);
extern Ball BallPosition(Ball ball);
extern void BallProps(SDL_Rect* ball_rect, Ball* ball);

// Utility
extern void CollisionCheck(Ball* ball, SDL_Rect* ball_rect,
                           Paddle* player1, Paddle* player2,
                           Mix_Chunk* pingHit);
extern void MovePaddle(Paddle* player);
extern void MoveBall(Ball* ball, SDL_Rect* ball_rect);
extern void SetScore(Paddle* player1, Paddle* player2);
extern void SetSpeed(Paddle* player1, Paddle* player2);

// Drawing
extern void BlitRect(SDL_Renderer* renderer, const SDL_Rect* rect, int c1, int c2, int c3);
extern void DrawFont(SDL_Texture* fontTexture, int x, int y, int c1, int c2, int c3);
extern SDL_Texture* LoadFont(char* filename, char buffer[]);

extern void GetInput(void);

App app;

Paddle* player1, *player2;