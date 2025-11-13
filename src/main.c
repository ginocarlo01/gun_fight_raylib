#include "raylib.h"
#include "types.h"
#include "audio.h"
#include "game_settings.h"
#include "entities.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <raymath.h>

const size_t OBSTACLES_QTY = 4;

typedef struct {
    Entity entities[64]; 
    int entities_qty;
    u8 player_score;
    u8 cpu_score;
} GameState;

void update_entity(Entity *entity){
    if(!entity->enabled) return;
    float deltaTime = GetFrameTime();
    entity->position = Vector2Add(entity->position, Vector2Scale(entity->direction, deltaTime * entity->speed));

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


void restart_game(GameState *game) {
    game->entities[0] = ENTITY_PLAYER;
    game->entities[1] = ENTITY_CPU;

    float start_pct = SCREEN_LIMIT_PLAYER * 0.01f;
    float end_pct = SCREEN_LIMIT_CPU * 0.01f;
    float step_pct = (end_pct - start_pct) / (OBSTACLES_QTY + 1);
    float center_y = SCREEN_DIMENSIONS.y * 0.5f;

    for (int k = 0; k < OBSTACLES_QTY; k++) {
        int i = 2 + k;
        game->entities[i] = OBSTACLES[k];
        game->entities[i].position = (Vector2){
            ((start_pct + step_pct * (k + 1)) * SCREEN_DIMENSIONS.x),
            center_y
        };
    }

    int next_index = 2 + OBSTACLES_QTY;
    for (int i = 0; i < ENTITY_PLAYER.ammo; i++) game->entities[next_index++] = ENTITY_BULLET_OF_PLAYER;
    for (int i = 0; i < ENTITY_CPU.ammo; i++) game->entities[next_index++] = ENTITY_BULLET_OF_CPU;

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
                    PlaySound(player_win_sfx);
                    restart_game(game);
                }
                if (entities[bullet_idx].owner == CPU && entities[target_idx].type == PLAYER) {
                    game->cpu_score++;
                    PlaySound(player_lose_sfx);
                    restart_game(game);
                }
                if (entities[target_idx].behaviour == DESTROY_BULLET_ONLY) {
                    PlaySound(ball_hit_sfx);
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

Vector2 get_player_input() {
    const Vector2 up    = { 0, -1 };
    const Vector2 down  = { 0,  1 };
    const Vector2 left  = { -1, 0 };
    const Vector2 right = { 1,  0 };

    Vector2 dir = Vector2Zero();

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dir = Vector2Add(dir, up);
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dir = Vector2Add(dir, down);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dir = Vector2Add(dir, left);
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir = Vector2Add(dir, right);

    return Vector2Normalize(dir);
}


int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_UNDECORATED);
    InitWindow(SCREEN_DIMENSIONS.x, SCREEN_DIMENSIONS.y, "");
    SetTargetFPS(TARGET_FPS);
    InitAudioAssets();
    GameState game = {0};
    restart_game(&game);
    game.player_score = 0;
    game.cpu_score = 0;
    Entity *player = &game.entities[0];
    Entity *cpu = &game.entities[1];
    cpu->direction = Vector2Normalize(cpu->direction);
    
    while (!WindowShouldClose()) {
        // //INPUT
        player->direction = get_player_input();
        if (IsKeyPressed(KEY_R)) restart_game(&game);
        if (IsKeyPressed(KEY_SPACE)) spawn_bullet(player, game.entities, game.entities_qty);
        

        //UPDATE
        auto_spawn_bullet(cpu, game.entities, game.entities_qty);
        for (int i = 0; i < game.entities_qty; i++) update_entity(&game.entities[i]);
        handle_bullet_collisions(&game);

        //DRAW
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        for (int i = 0; i < game.entities_qty; i++) draw_entity(game.entities[i]);
        
        //DEBUG
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);
        DrawText(TextFormat("Player Score: %i", game.player_score), 10, 50, 20, WHITE);
        DrawText(TextFormat("CPU Score: %i", game.cpu_score), 10, 70, 20, WHITE);
        DrawText(TextFormat("Player Ammo: %i", player->ammo), 10, 90, 20, WHITE);

        EndDrawing();
    }
    UnloadAudioAssets();
    CloseWindow();
    return 0;
}