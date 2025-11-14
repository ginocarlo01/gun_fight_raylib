#include "game_logic.h"
#include "audio.h"
#include "game_settings.h"
#include "entities.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const size_t OBSTACLES_QTY = 4;

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

void spawn_bullet(Entity *entity, Entity *entities, int entities_qty){
    if(entity->ammo <= 0) return;
    for(int i = entities_qty-1; i >= 0; i--){
        if(entities[i].owner != entity->type) continue;
        if(entities[i].position.x != 0 && entities[i].position.y != 0) continue;
        if(entities[i].enabled) continue;
        entities[i].enabled = true;
        entities[i].position = (Vector2){entity->position.x + (entity->radius + 10) * (entity->type == PLAYER ? 1 : -1), entity->position.y};
        entities[i].direction.x = (entity->type == PLAYER ? 1 : -1);
        entity->ammo--;
        return;
    }
}

void auto_spawn_bullet(Entity *entity, Entity *entities, int entities_qty){
    static float recharge_time_counter = 0;
    recharge_time_counter += GetFrameTime();
    if(recharge_time_counter > entity->recharge_time) {
        spawn_bullet(entity, entities, entities_qty);
        recharge_time_counter = 0;
    }
}

void restart_game(GameState *game) {
    game->entities[0] = DefaultPlayer;
    game->entities[1] = DefaultCPU;

    float start_pct = ScreenLimitPlayer * 0.01f;
    float end_pct = ScreenLimitCPU * 0.01f;
    float step_pct = (end_pct - start_pct) / (OBSTACLES_QTY + 1);
    float center_y = ScreenDimensions.y * 0.5f;

    for (int k = 0; k < OBSTACLES_QTY; k++) {
        int i = 2 + k;
        game->entities[i] = ObstaclesOrder[k];
        game->entities[i].position = (Vector2){
            ((start_pct + step_pct * (k + 1)) * ScreenDimensions.x),
            center_y
        };
    }

    int next_index = 2 + OBSTACLES_QTY;
    for (int i = 0; i < DefaultPlayer.ammo; i++) game->entities[next_index++] = DefaultBulletOfPlayer;
    for (int i = 0; i < DefaultCPU.ammo; i++) game->entities[next_index++] = DefaultBulletOfCPU;

    game->entities_qty = next_index;
}

void handle_bullet_collisions(GameState *game) {
    Entity *entities = game->entities;

    for (int bullet_idx = 2 + OBSTACLES_QTY; bullet_idx < game->entities_qty; bullet_idx++) {
        if (!entities[bullet_idx].enabled) continue;

        for (int target_idx = 0; target_idx < 2 + OBSTACLES_QTY; target_idx++) {
            if (!entities[target_idx].enabled) continue;

            if (CheckCollisionCircles(entities[bullet_idx].position, entities[bullet_idx].radius,entities[target_idx].position, entities[target_idx].radius)) {

                if (entities[bullet_idx].owner == PLAYER && entities[target_idx].type == CPU) {
                    game->player_score++;
                    PlaySound(PlayerWinSFX);
                    restart_game(game);
                    return;
                }
                if (entities[bullet_idx].owner == CPU && entities[target_idx].type == PLAYER) {
                    game->cpu_score++;
                    PlaySound(PlayerLoseSFX);
                    restart_game(game);
                    return;
                }
                if (entities[target_idx].behaviour == DESTROY_BULLET_ONLY) {
                    PlaySound(BallHitSFX);
                    entities[bullet_idx].enabled = entities[target_idx].enabled = false;
                }
                if (entities[target_idx].behaviour == DAMAGE_OWNER) {
                    (entities[bullet_idx].owner == PLAYER ? game->cpu_score++ : game->player_score++);
                    restart_game(game);
                }
            }
        }
    }
}

Vector2 process_input(GameState *game, InputPacket input) {
    const Vector2 up    = { 0, -1 };
    const Vector2 down  = { 0,  1 };
    const Vector2 left  = { -1, 0 };
    const Vector2 right = { 1,  0 };

    Vector2 dir = Vector2Zero();

    if (input.up)    dir = Vector2Add(dir, up);
    if (input.down)  dir = Vector2Add(dir, down);
    if (input.left)  dir = Vector2Add(dir, left);
    if (input.right) dir = Vector2Add(dir, right);

    if (input.shoot) spawn_bullet(&((game->entities)[0]), game->entities, game->entities_qty);

    return Vector2Normalize(dir);
}
