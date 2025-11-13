#include "entities.h"
#include <raylib.h>

const Entity ENTITY_PLAYER = { .type = PLAYER, .position = {100, 270}, .color = BLUE, .speed = 200, .radius = 20, .ammo = 5, .enabled = true };
const Entity ENTITY_CPU = { .type = CPU, .position = {860, 270}, .direction = {1, 1}, .color = RED, .speed = 150, .radius = 30, .ammo = 5, .recharge_time = 1, .enabled = true };
const Entity ENTITY_BULLET_OF_PLAYER = { .type = BULLET, .color = BLUE, .speed = 300, .radius = 10, .owner = PLAYER };
const Entity ENTITY_BULLET_OF_CPU = { .type = BULLET, .color = RED, .speed = 300, .radius = 10, .owner = CPU };
const Entity ENTITY_OBSTACLE_DESTROY_BULLET_ONLY = { .type = OBSTACLE, .behaviour = DESTROY_BULLET_ONLY, .direction = {0, 1}, .color = WHITE, .speed = 150, .radius = 15, .enabled = true };
const Entity ENTITY_OBSTACLE_DAMAGE_OWNER = { .type = OBSTACLE, .behaviour = DAMAGE_OWNER, .direction = {0, 1}, .color = RED, .speed = 200, .radius = 15, .enabled = true };
const Entity OBSTACLES[] = { ENTITY_OBSTACLE_DESTROY_BULLET_ONLY, ENTITY_OBSTACLE_DESTROY_BULLET_ONLY, ENTITY_OBSTACLE_DESTROY_BULLET_ONLY, ENTITY_OBSTACLE_DESTROY_BULLET_ONLY };
