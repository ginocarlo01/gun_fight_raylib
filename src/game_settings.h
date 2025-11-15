#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include "types.h"
#include <raylib.h>

#define SERVER_PORT 2000
#define MAX_PLAYERS_ONLINE_MODE 2

extern const Vec2 ScreenDimensions;
extern const u8 TargetFPS;
extern const Color BackgroundColor;
extern const percentage ScreenLimitPlayer;
extern const percentage ScreenLimitCPU;

#endif