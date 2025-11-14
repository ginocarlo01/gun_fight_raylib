#include "game_logic.h"
#include "audio.h"
#include "game_settings.h"
#include "entities.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

void update_entity(Entity *entity, float delta_time){
    if(!entity->enabled) return;
    entity->position = Vector2Add(entity->position, Vector2Scale(entity->direction, delta_time * entity->speed));
    
    switch (entity->type)
    {
    case OBSTACLE:
        if(entity->position.x - entity->radius < ScreenLimitCPU * ScreenDimensions.x * 0.01) entity->direction.x *= -1;
        if(entity->position.x + entity->radius > ScreenDimensions.x) entity->direction.x *= -1;
        if(entity->position.y + entity->radius > ScreenDimensions.y) entity->direction.y *= -1;
        if(entity->position.y - entity->radius <= 0) entity->direction.y *= -1;
        break;
    case CPU:
        if(entity->position.x - entity->radius < ScreenLimitCPU * ScreenDimensions.x * 0.01) entity->direction.x *= -1;
        if( entity->position.x + entity->radius > ScreenDimensions.x) entity->direction.x *= -1;
        if(entity->position.y + entity->radius > ScreenDimensions.y) entity->direction.y *= -1;
        if(entity->position.y - entity->radius <= 0) entity->direction.y *= -1;
        break;
    case PLAYER:
        if(entity->position.x + entity->radius > ScreenLimitPlayer * ScreenDimensions.x * 0.01) entity->position.x = ScreenLimitPlayer * ScreenDimensions.x * 0.01 - entity->radius;
        if( entity->position.x - entity->radius <= 0) entity->position.x = entity->radius;
        if(entity->position.y + entity->radius > ScreenDimensions.y) entity->position.y = ScreenDimensions.y - entity->radius;
        if(entity->position.y - entity->radius <= 0) entity->position.y = entity->radius;
        break;
    case BULLET:
        if(entity->position.x  > ScreenDimensions.x || entity->position.x  <= 0) entity->enabled = false;
        if(entity->position.y > ScreenDimensions.y || entity->position.y  <= 0) entity->direction.y *= -1; 
        break;
    default:
        break;
    }
}

void draw_entity(Entity entity){
    if(!entity.enabled) return;
    DrawCircle(entity.position.x, entity.position.y, entity.radius, entity.color);
}

void spawn_bullet(Entity *entity, GameState *game_state){
    if(entity->ammo <= 0) return;
    for(int i = game_state->entities_qty-1; i >= 0; i--){
        if(game_state->entities[i].owner != entity->type) continue;
        if(game_state->entities[i].position.x != 0 && game_state->entities[i].position.y != 0) continue;
        if(game_state->entities[i].enabled) continue;
        game_state->entities[i].enabled = true;
        game_state->entities[i].position = (Vector2){entity->position.x + (entity->radius + 10) * (entity->type == PLAYER ? 1 : -1), entity->position.y};
        game_state->entities[i].direction.x = (entity->type == PLAYER ? 1 : -1);
        entity->ammo--;
        return;
    }
}

void collect_ammo(Entity *entity, GameState *game_state){
    for(int i = game_state->entities_qty-1; i >= ObstaclesOrderSize; i--){
        if(game_state->entities[i].owner != entity->type) continue;
        if(game_state->entities[i].enabled) continue;
        if(game_state->entities[i].position.x != 0 && game_state->entities[i].position.y != 0){
            game_state->entities[i].position = Vector2Zero();
            entity->ammo++;
            return;
        }
    }
}

void auto_spawn_bullet(Entity *entity, GameState *game_state){
    static float recharge_time_counter = 0;
    recharge_time_counter += GetFrameTime();
    if(recharge_time_counter > entity->recharge_time) {
        spawn_bullet(entity, game_state);
        recharge_time_counter = 0;
    }
}

void restart_game(GameState *game_state) {
    game_state->entities[0] = DefaultPlayer;
    game_state->entities[1] = DefaultCPU;

    float start_pct = ScreenLimitPlayer * 0.01f;
    float end_pct = ScreenLimitCPU * 0.01f;
    float step_pct = (end_pct - start_pct) / (ObstaclesOrderSize + 1);
    float center_y = ScreenDimensions.y * 0.5f;

    for (int k = 0; k < ObstaclesOrderSize; k++) {
        int i = 2 + k;
        game_state->entities[i] = ObstaclesOrder[k];
        game_state->entities[i].position = (Vector2){
            ((start_pct + step_pct * (k + 1)) * ScreenDimensions.x),
            center_y
        };
    }

    int next_index = 2 + ObstaclesOrderSize;
    for (int i = 0; i < DefaultPlayer.ammo; i++) game_state->entities[next_index++] = DefaultBulletOfPlayer;
    for (int i = 0; i < DefaultCPU.ammo; i++) game_state->entities[next_index++] = DefaultBulletOfCPU;

    game_state->entities_qty = next_index;
}

void handle_bullet_collisions(GameState *game_state) {
    Entity *entities = game_state->entities;

    for (int bullet_idx = 2 + ObstaclesOrderSize; bullet_idx < game_state->entities_qty; bullet_idx++) {
        if (!entities[bullet_idx].enabled) continue;

        for (int target_idx = 0; target_idx < 2 + ObstaclesOrderSize; target_idx++) {
            if (!entities[target_idx].enabled) continue;

            if (CheckCollisionCircles(entities[bullet_idx].position, entities[bullet_idx].radius,entities[target_idx].position, entities[target_idx].radius)) {

                if (entities[bullet_idx].owner == PLAYER && entities[target_idx].type == CPU) {
                    game_state->player_score++;
                    PlaySound(PlayerWinSFX);
                    restart_game(game_state);
                    return;
                }
                if (entities[bullet_idx].owner == CPU && entities[target_idx].type == PLAYER) {
                    game_state->cpu_score++;
                    PlaySound(PlayerLoseSFX);
                    restart_game(game_state);
                    return;
                }
                if (entities[target_idx].behaviour == DESTROY_BULLET_ONLY) {
                    PlaySound(BallHitSFX);
                    entities[bullet_idx].enabled = entities[target_idx].enabled = false;
                }
                if (entities[target_idx].behaviour == GIVE_AMMO_OWNER) {
                    PlaySound(BallHitSFX);
                    collect_ammo(entities[bullet_idx].owner == PLAYER ? &game_state->entities[0] : &game_state->entities[1],game_state);
                    entities[bullet_idx].enabled = entities[target_idx].enabled = false;
                }

                if (entities[target_idx].behaviour == DAMAGE_OWNER) {
                    (entities[bullet_idx].owner == PLAYER ? game_state->cpu_score++ : game_state->player_score++);
                    restart_game(game_state);
                }
            }
        }
    }
}

Vector2 process_input(EntityType entity_type, GameState *game_state, InputPacket input) {
    const Vector2 up    = { 0, -1 };
    const Vector2 down  = { 0,  1 };
    const Vector2 left  = { -1, 0 };
    const Vector2 right = { 1,  0 };

    Vector2 dir = Vector2Zero();

    if (input.up)    dir = Vector2Add(dir, up);
    if (input.down)  dir = Vector2Add(dir, down);
    if (input.left)  dir = Vector2Add(dir, left);
    if (input.right) dir = Vector2Add(dir, right);

    if (input.shoot) spawn_bullet(&((game_state->entities)[entity_type == PLAYER ? 0 : 1]), game_state);

    return Vector2Normalize(dir);
}
