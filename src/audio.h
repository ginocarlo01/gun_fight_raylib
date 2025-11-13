#ifndef AUDIO_H
#define AUDIO_H

#include <raylib.h>

extern const char* ball_hit_sfx_path;
extern const char* player_win_sfx_path;
extern const char* player_lose_sfx_path;

extern Sound ball_hit_sfx;
extern Sound player_win_sfx;
extern Sound player_lose_sfx;

void InitAudioAssets();
void UnloadAudioAssets();

#endif
