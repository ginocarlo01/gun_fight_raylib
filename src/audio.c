#include "audio.h"

const char* ball_hit_sfx_path   = "sfx/ball_hit.wav";
const char* player_win_sfx_path = "sfx/win.wav";
const char* player_lose_sfx_path = "sfx/lose.wav";

Sound ball_hit_sfx;
Sound player_win_sfx;
Sound player_lose_sfx;

void InitAudioAssets() {
    InitAudioDevice(); 
    ball_hit_sfx   = LoadSound(ball_hit_sfx_path);
    player_win_sfx = LoadSound(player_win_sfx_path);
    player_lose_sfx = LoadSound(player_lose_sfx_path);
}

void UnloadAudioAssets() {
    UnloadSound(ball_hit_sfx);
    UnloadSound(player_win_sfx);
    UnloadSound(player_lose_sfx);
    CloseAudioDevice();
}
