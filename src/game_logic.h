#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "types.h"

void update_entity(Entity *entity, float delta_time);
void draw_entity(Entity entity);
void spawn_bullet(Entity *entity, GameState *game_state);
void collect_ammo(Entity *entity, GameState *game_state);
void auto_spawn_bullet(Entity *entity, GameState *game_state);
void restart_game(GameState *game);
void check_end_of_match(GameState *game);
void handle_bullet_collisions(GameState *game);
Vector2 process_input(EntityType entity_type, GameState *game, InputPacket input) ;

#endif

