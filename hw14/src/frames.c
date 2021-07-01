#include "common.h"

void CapFramerate(long* then, float* remainder) {
    long wait, frame_time;
    wait = 16 + *remainder;
    *remainder -= (int) *remainder;
    frame_time = SDL_GetTicks() - *then;
    wait -= frame_time;

    if (wait < 1) {
        wait = 1;
    }

    SDL_Delay(wait);
    *remainder += 0.667;
    *then = SDL_GetTicks();
}