#include "raylib.h"
#include "types.h"
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const size_t OBSTACLES_QTY = sizeof(OBSTACLES) / sizeof(OBSTACLES[0]);

void normalize(Vector2 *v) {
    if (!v) return;
    float mag = sqrtf(v->x * v->x + v->y * v->y);
    if (mag != 0.0f) {
        v->x /= mag;
        v->y /= mag;
    }
}

void audio_init() {
    InitAudioDevice();
    ball_hit_sfx   = LoadSound(ball_hit_sfx_path);
    player_win_sfx = LoadSound(player_win_sfx_path);
    player_lose_sfx = LoadSound(player_lose_sfx_path);
}

void audio_unload() {
    UnloadSound(ball_hit_sfx);
    UnloadSound(player_win_sfx);
    UnloadSound(player_lose_sfx);
    CloseAudioDevice();
}

void update_entity(Entity *entity){
    if(!entity->enabled) return;
    float deltaTime = GetFrameTime();
    entity->position.x += deltaTime * entity->speed * entity->direction.x;
    entity->position.y += deltaTime * entity->speed * entity->direction.y;

    switch (entity->type)
    {
    case OBSTACLE:
        if(entity->position.x - entity->radius < SCREEN_LIMIT_CPU * SCREEN_DIMENSIONS.x * 0.01) entity->direction.x *= -1;
        if(entity->position.x + entity->radius > SCREEN_DIMENSIONS.x) entity->direction.x *= -1;
        if(entity->position.y + entity->radius > SCREEN_DIMENSIONS.y) entity->direction.y *= -1;
        if(entity->position.y - entity->radius <= 0) entity->direction.y *= -1;
        break;
    case CPU:
        if(entity->position.x - entity->radius < SCREEN_LIMIT_CPU * SCREEN_DIMENSIONS.x * 0.01) entity->direction.x *= -1;
        if( entity->position.x + entity->radius > SCREEN_DIMENSIONS.x) entity->direction.x *= -1;
        if(entity->position.y + entity->radius > SCREEN_DIMENSIONS.y) entity->direction.y *= -1;
        if(entity->position.y - entity->radius <= 0) entity->direction.y *= -1;
        break;
    case PLAYER:
        if(entity->position.x + entity->radius > SCREEN_LIMIT_PLAYER * SCREEN_DIMENSIONS.x * 0.01) entity->position.x = SCREEN_LIMIT_PLAYER * SCREEN_DIMENSIONS.x * 0.01 - entity->radius;
        if( entity->position.x - entity->radius <= 0) entity->position.x = entity->radius;
        if(entity->position.y + entity->radius > SCREEN_DIMENSIONS.y) entity->position.y = SCREEN_DIMENSIONS.y - entity->radius;
        if(entity->position.y - entity->radius <= 0) entity->position.y = entity->radius;
        break;
    case BULLET:
        if(entity->position.x  > SCREEN_DIMENSIONS.x || entity->position.x  <= 0) entity->enabled = false;
        if(entity->position.y > SCREEN_DIMENSIONS.y || entity->position.y  <= 0) entity->direction.y *= -1; 
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

void restart_game(Entity *entities) {
    entities[0] = ENTITY_PLAYER;
    entities[1] = ENTITY_CPU;

    float start_pct = SCREEN_LIMIT_PLAYER * 0.01f;
    float end_pct = SCREEN_LIMIT_CPU * 0.01f;
    float step_pct = (end_pct - start_pct) / (OBSTACLES_QTY + 1);
    float center_y = SCREEN_DIMENSIONS.y * 0.5f;

    for (int k = 0; k < OBSTACLES_QTY; k++) {
        int i = 2 + k;
        entities[i] = OBSTACLES[k];
        entities[i].position.y = center_y;
        entities[i].position.x = (start_pct + step_pct * (k + 1)) * SCREEN_DIMENSIONS.x;
    }

    int next_index = 2 + OBSTACLES_QTY;
    for (int i = 0; i < entities[0].ammo; i++) entities[next_index++] = ENTITY_BULLET_OF_PLAYER;
    for (int i = 0; i < entities[1].ammo; i++) entities[next_index++] = ENTITY_BULLET_OF_CPU;
}


void handle_bullet_collisions(Entity *entities, int entities_qty, u8 *player_score, u8 *cpu_score){
    for (int bullet_idx = 2 + OBSTACLES_QTY; bullet_idx < entities_qty; bullet_idx++) {
        if(!entities[bullet_idx].enabled) continue;
        for(int target_idx = 0; target_idx < 2 + OBSTACLES_QTY;target_idx++){
            if(!entities[target_idx].enabled) continue;
            if(CheckCollisionCircles(entities[bullet_idx].position, entities[bullet_idx].radius, entities[target_idx].position, entities[target_idx].radius)){
                if(entities[bullet_idx].owner == PLAYER && entities[target_idx].type == CPU) {(*player_score)++; PlaySound(player_win_sfx);restart_game(entities);}
                if(entities[bullet_idx].owner == CPU && entities[target_idx].type == PLAYER) {(*cpu_score)++; PlaySound(player_lose_sfx); restart_game(entities);}
                if(entities[target_idx].behaviour == DESTROY_BULLET_ONLY) {PlaySound(ball_hit_sfx); entities[bullet_idx].enabled = entities[target_idx].enabled = false;}
                if(entities[target_idx].behaviour == DAMAGE_OWNER) {(entities[bullet_idx].owner == PLAYER? (*cpu_score)++ : (*player_score)++); restart_game(entities);}
            }
        }
    }
}

int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_UNDECORATED);
    InitWindow(SCREEN_DIMENSIONS.x, SCREEN_DIMENSIONS.y, "");
    SetTargetFPS(TARGET_FPS);
    audio_init();
    int entities_qty = ENTITY_PLAYER.ammo + ENTITY_CPU.ammo + OBSTACLES_QTY + 2;
    Entity entities[entities_qty];
    restart_game(entities);
    Entity *player = &entities[0];
    Entity *cpu = &entities[1];
    normalize(&(*cpu).direction);
    u8 player_score;
    u8 cpu_score;
    
    while (!WindowShouldClose()) {
        //INPUT
        (*player).direction = (Vector2){0,0};
        if(IsKeyDown(KEY_UP)) (*player).direction.y = -1;
        if(IsKeyDown(KEY_DOWN)) (*player).direction.y = 1;
        if(IsKeyDown(KEY_RIGHT)) (*player).direction.x = 1;
        if(IsKeyDown(KEY_LEFT)) (*player).direction.x = -1;
        if(IsKeyPressed(KEY_R)) restart_game(entities);
        if(IsKeyPressed(KEY_SPACE)) spawn_bullet(&(*player), entities, entities_qty);
        normalize(&(*player).direction);

        //UPDATE
        auto_spawn_bullet(&(*cpu), entities, entities_qty);
        for (u8 i = 0; i < entities_qty; i++) update_entity(&entities[i]);
        handle_bullet_collisions(entities, entities_qty, &player_score, &cpu_score);

        //DRAW
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        for (u8 i = 0; i < entities_qty; i++) draw_entity(entities[i]);
        
        //DEBUG
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);
        DrawText(TextFormat("Player Score: %i", player_score), 10, 50, 20, WHITE);
        DrawText(TextFormat("CPU Score: %i", cpu_score), 10, 70, 20, WHITE);
        DrawText(TextFormat("Player Ammo: %i", player->ammo), 10, 90, 20, WHITE);

        EndDrawing();
    }
    audio_unload();
    CloseWindow();
    return 0;
}