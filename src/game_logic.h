#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "types.h"

void update_entity(Entity *entity, float delta_time);
void draw_entity(Entity entity);
void spawn_bullet(Entity *entity, Entity *entities, int entities_qty);
void auto_spawn_bullet(Entity *entity, Entity *entities, int entities_qty);
void restart_game(GameState *game);
void handle_bullet_collisions(GameState *game);
Vector2 process_input(EntityType entity_type, GameState *game, InputPacket input) ;

#endif

