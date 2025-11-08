#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

const uint16_t SCREEN_WIDTH = 960;
const uint16_t SCREEN_HEIGHT = 540;
const uint16_t HALF_SCREEN_WIDTH = SCREEN_WIDTH * 0.5;
const uint16_t HALF_SCREEN_HEIGHT = SCREEN_HEIGHT * 0.5;
const char* GAME_TITLE = "Game Title";
const uint8_t TARGET_FPS = 240;
const Color BACKGROUND_COLOR = BLACK;

const uint8_t PLAYER_BULLETS_QTY = 5;
const uint8_t CPU_BULLETS_QTY = 5;
const uint8_t ENTITIES_QTY = PLAYER_BULLETS_QTY + CPU_BULLETS_QTY + 2;

typedef enum{
    CPU,
    PLAYER,
    BULLET
} EntityType;

//typedef struct Entity Entity;

typedef struct Entity {
    EntityType type;
    Vector2 position;
    Vector2 direction;
    uint8_t radius;
    EntityType owner;
    uint16_t speed;
    Color color;
    bool enabled;
} Entity;

void normalize(Vector2 *v) {
    if (!v) return;
    float mag = sqrtf(v->x * v->x + v->y * v->y);
    if (mag != 0.0f) {
        v->x /= mag;
        v->y /= mag;
    }
}

void update_entity(Entity *entity){
    if(!entity->enabled) return;
    float deltaTime = GetFrameTime();
    entity->position.x += deltaTime * entity->speed * entity->direction.x;
    entity->position.y += deltaTime * entity->speed * entity->direction.y;

}

void draw_entity(Entity entity){
    if(!entity.enabled) return;
    DrawCircle(entity.position.x, entity.position.y, entity.radius, entity.color);
}

void spawn_bullet(Entity entity, Entity *entities){
    //TODO
    for(uint8_t i = 5; i > 0; i--){
        printf("%d\n", i);
        if(!entities[i].owner) continue;
        if(entities[i].owner != entity.type) continue;
        if(entities[i].position.x != 0 && entities[i].position.y != 0) continue;
        if(entities[i].enabled) continue;
        entities[i].enabled = true;
        entities[i].position.x = entity.position.x + 30;
        entities[i].position.y = entity.position.y;
        entities[i].direction.x = 1;
        return;
    }
}

int main(){
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    //Entity player = {.type = PLAYER, .position = (Vector2){HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT}, .radius = 20, .speed = 200, .color = WHITE,.enabled = true};
    
    Entity entities[ENTITIES_QTY];
    entities[0] = (Entity){.type = PLAYER, .position = (Vector2){HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT}, .radius = 20, .speed = 200, .color = WHITE,.enabled = true};
    Entity *player = &entities[0];
    for (uint8_t i = 1; i < PLAYER_BULLETS_QTY+1; ++i) entities[i] = (Entity){.type = BULLET,.position = (Vector2){0, 0},.direction = (Vector2){0, 0},.radius = 10, .speed = 300 ,.color = BLUE,.enabled = false,.owner = PLAYER};
    
    
    while (!WindowShouldClose()) {
        //INPUT
        (*player).direction = (Vector2){0,0};
        if(IsKeyDown(KEY_UP)) (*player).direction.y = -1;
        if(IsKeyDown(KEY_DOWN)) (*player).direction.y = 1;
        if(IsKeyDown(KEY_RIGHT)) (*player).direction.x = 1;
        if(IsKeyDown(KEY_LEFT)) (*player).direction.x = -1;
        if(IsKeyPressed(KEY_SPACE)) spawn_bullet((*player), entities);
        normalize(&(*player).direction);

        //UPDATE
        
        for (uint8_t i = 0; i < PLAYER_BULLETS_QTY+1; i++) update_entity(&entities[i]);

        //DRAW
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        //draw_entity(player);
        for (uint8_t i = 0; i < PLAYER_BULLETS_QTY+1; i++) draw_entity(entities[i]);
        //TODO
        // for(iterator = 0; iterator < ENTITIES_QTY; iterator++){
        // }

        //DEBUG
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}