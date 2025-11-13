#ifndef COMMON_H
#define COMMON_H

#include "types.h"
#include "config.h"
#include "raylib.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string.h>

extern const size_t OBSTACLES_QTY;

void normalize(Vector2 *v);
void update_entity(Entity *entity, float delta);
void spawn_bullet(Entity *entity, Entity *entities, int entities_qty);
void restart_game(Entity *entities);
void handle_bullet_collisions(Entity *entities, int entities_qty, u8 *player_score, u8 *cpu_score);

#endif
