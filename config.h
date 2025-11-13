#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

#define SERVER_PORT 2000
#define MAX_ENTITIES 16

extern const Vec2 SCREEN_DIMENSIONS;
extern const u8 TARGET_FPS;
extern const Color BACKGROUND_COLOR;
extern const percentage SCREEN_LIMIT_PLAYER;
extern const percentage SCREEN_LIMIT_CPU;

extern const Entity ENTITY_PLAYER;
extern const Entity ENTITY_CPU;
extern const Entity ENTITY_BULLET_OF_PLAYER;
extern const Entity ENTITY_BULLET_OF_CPU;
extern const Entity ENTITY_OBSTACLE_DESTROY_BULLET_ONLY;
extern const Entity ENTITY_OBSTACLE_DAMAGE_OWNER;
extern const Entity OBSTACLES[];

extern const char* ball_hit_sfx_path;
extern const char* player_win_sfx_path;
extern const char* player_lose_sfx_path;

extern Sound ball_hit_sfx;
extern Sound player_win_sfx;
extern Sound player_lose_sfx;

#endif
