#ifndef CONFIG_H
#define CONFIG_H

const Vec2 SCREEN_DIMENSIONS = {960, 540};
const u8 TARGET_FPS = 240;
const Color BACKGROUND_COLOR = BLACK;
const percentage SCREEN_LIMIT_PLAYER = 30;
const percentage SCREEN_LIMIT_CPU = 70;
const Entity ENTITY_PLAYER = {.type = PLAYER,.position = (Vector2){100, 270}, .color = BLUE, .speed = 200, .radius = 20, .ammo = 5, .enabled = true};
const Entity ENTITY_CPU = {.type = CPU,.position = (Vector2){860, 270}, .direction = {1,1} ,.color = RED, .speed = 150, .radius = 30, .ammo = 5, .recharge_time = 1,.enabled = true};
const Entity ENTITY_BULLET_OF_PLAYER = {.type = BULLET,.color = BLUE, .speed = 300, .radius = 10, .owner = PLAYER};
const Entity ENTITY_BULLET_OF_CPU = {.type = BULLET,.color = RED, .speed = 300, .radius = 10, .owner = CPU};
const Entity ENTITY_OBSTACLE_DESTROY_BULLET_ONLY = {.type = OBSTACLE, .behaviour = DESTROY_BULLET_ONLY, .direction = {0,1} ,.color = WHITE, .speed = 150, .radius = 15, .enabled = true};
const Entity ENTITY_OBSTACLE_DAMAGE_OWNER = {.type = OBSTACLE, .behaviour = DAMAGE_OWNER, .direction = {0,1} ,.color = RED, .speed = 200, .radius = 15, .enabled = true};
const Entity OBSTACLES[] = {ENTITY_OBSTACLE_DESTROY_BULLET_ONLY, ENTITY_OBSTACLE_DESTROY_BULLET_ONLY, ENTITY_OBSTACLE_DESTROY_BULLET_ONLY,ENTITY_OBSTACLE_DESTROY_BULLET_ONLY};
const char* ball_hit_sfx_path = "sfx/ball_hit.wav";
const char* player_win_sfx_path = "sfx/win.wav";
const char* player_lose_sfx_path = "sfx/lose.wav";
Sound ball_hit_sfx;
Sound player_win_sfx;
Sound player_lose_sfx;
#endif