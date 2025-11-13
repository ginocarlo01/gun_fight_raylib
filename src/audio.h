#ifndef AUDIO_H
#define AUDIO_H

#include <raylib.h>

extern const char* BallHitSFXPath;
extern const char* PlayerWinSFXPath;
extern const char* PlayerLoseSFXPath;

extern Sound BallHitSFX;
extern Sound PlayerWinSFX;
extern Sound PlayerLoseSFX;

void init_audio();
void close_audio();

#endif
