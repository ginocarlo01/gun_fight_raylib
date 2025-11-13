#include "audio.h"

const char* BallHitSFXPath   = "sfx/ball_hit.wav";
const char* PlayerWinSFXPath = "sfx/win.wav";
const char* PlayerLoseSFXPath = "sfx/lose.wav";

Sound BallHitSFX;
Sound PlayerWinSFX;
Sound PlayerLoseSFX;

void init_audio() {
    InitAudioDevice(); 
    BallHitSFX   = LoadSound(BallHitSFXPath);
    PlayerWinSFX = LoadSound(PlayerWinSFXPath);
    PlayerLoseSFX = LoadSound(PlayerLoseSFXPath);
}

void close_audio() {
    UnloadSound(BallHitSFX);
    UnloadSound(PlayerWinSFX);
    UnloadSound(PlayerLoseSFX);
    CloseAudioDevice();
}
