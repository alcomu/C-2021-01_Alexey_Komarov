#include "sound.h"

Mix_Chunk* LoadSoundEffect(char* filePath) {
    Mix_Chunk* soundEffect;
    soundEffect = Mix_LoadWAV(filePath);

    if (soundEffect == NULL) {
        fprintf(stderr, "Failed to load %s sound effect: %s.", filePath, Mix_GetError());
        exit(1);
    }

    return soundEffect;
}

void CleanupAudio(Mix_Chunk* soundEffect) {
    Mix_FreeChunk(soundEffect);
    soundEffect = NULL;
    Mix_Quit();
}