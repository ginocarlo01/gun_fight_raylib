#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t percentage;
typedef uint8_t seconds;
typedef struct Vec2 {uint16_t x;uint16_t y;} Vec2;

const Vec2 SCREEN_DIMENSIONS = {960, 540};
const uint8_t TARGET_FPS = 240;
const char* GAME_TITLE = "Game Title";
const Color BACKGROUND_COLOR = BLACK;
const percentage SCREEN_LIMIT_PLAYER = 30;
const percentage SCREEN_LIMIT_CPU = 70;

typedef enum{CPU,PLAYER,BULLET, OBSTACLE} EntityType;
typedef enum{DAMAGE_OWNER, LIFE_OWNER, DESTROY_BULLET} EntityBehaviour;
typedef struct Entity {EntityType type;Vector2 position;Vector2 direction;uint8_t radius;uint8_t ammo;seconds recharge_time;EntityBehaviour behaviour;EntityType owner;uint16_t speed; Color color;bool enabled;} Entity;

//TODO change the init values for percentage
const Entity ENTITY_PLAYER = {.type = PLAYER,.position = (Vector2){100, 270}, .color = BLUE, .speed = 200, .radius = 20, .ammo = 5,.enabled = true};
const Entity ENTITY_CPU = {.type = CPU,.position = (Vector2){860, 270}, .direction = {1,1} ,.color = RED, .speed = 150, .radius = 30, .ammo = 5, .recharge_time = 1,.enabled = true};
const Entity ENTITY_BULLET_OF_PLAYER = {.type = BULLET,.color = BLUE, .speed = 300, .radius = 10, .owner = PLAYER};
const Entity ENTITY_BULLET_OF_CPU = {.type = BULLET,.color = RED, .speed = 300, .radius = 10, .owner = CPU};

//TODO reorganize
const Entity OBSTACLES[] = {
    {.type = OBSTACLE, .behaviour = DESTROY_BULLET, .direction = {0,1} ,.color = WHITE, .speed = 150, .radius = 15, .enabled = true},
    {.type = OBSTACLE, .behaviour = DAMAGE_OWNER, .direction = {0,1} ,.color = RED, .speed = 150, .radius = 15, .enabled = true},
    {.type = OBSTACLE, .behaviour = LIFE_OWNER, .direction = {0,1} ,.color = GREEN, .speed = 150, .radius = 15, .enabled = true},
    {.type = OBSTACLE, .behaviour = DESTROY_BULLET, .direction = {0,1} ,.color = WHITE, .speed = 150, .radius = 15, .enabled = true}
};
const size_t OBSTACLES_QTY = sizeof(OBSTACLES) / sizeof(OBSTACLES[0]);

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

    switch (entity->type)
    {
    case OBSTACLE:
        if(entity->position.x - entity->radius < SCREEN_LIMIT_CPU * SCREEN_DIMENSIONS.x * 0.01) entity->direction.x *= -1;
        if( entity->position.x + entity->radius > SCREEN_DIMENSIONS.x) entity->direction.x *= -1;
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

void spawn_bullet(Entity entity, Entity *entities, int entities_qty){
    if(entity.ammo <= 0) return;
    for(int i = entities_qty-1; i > 1 + OBSTACLES_QTY; i--){
        if(entities[i].owner != entity.type) continue;
        if(entities[i].position.x != 0 && entities[i].position.y != 0) continue;
        if(entities[i].enabled) continue;
        entities[i].enabled = true;
        entities[i].position = (Vector2){entity.position.x + 30 * (entity.type == PLAYER ? 1 : -1), entity.position.y};
        entities[i].direction.x = (entity.type == PLAYER ? 1 : -1);
        return;
    }
}

void auto_spawn_bullet(Entity entity, Entity *entities, int entities_qty){
    static float recharge_time_count_down = 0;
    recharge_time_count_down += GetFrameTime();
    if(recharge_time_count_down > entity.recharge_time) {
        spawn_bullet(entity, entities, entities_qty);
        recharge_time_count_down = 0;
    }
}

void restart_game(Entity *entities){
    entities[0] = ENTITY_PLAYER;
    entities[1] = ENTITY_CPU;
    //TODO REORGANIZE
    for (int k = 0; k < OBSTACLES_QTY; k++) {
        int i = 2 + k;
        entities[i] = OBSTACLES[k];
        entities[i].position.y = SCREEN_DIMENSIONS.y * 0.5;
        entities[i].position.x = ((SCREEN_LIMIT_CPU - SCREEN_LIMIT_PLAYER)*0.01/(OBSTACLES_QTY+1) * (k+1)+ SCREEN_LIMIT_PLAYER*0.01)* SCREEN_DIMENSIONS.x; 
    } 
    
    for (uint8_t i = 0; i < entities[0].ammo; i++) entities[i+2+OBSTACLES_QTY] = ENTITY_BULLET_OF_PLAYER;
    for (uint8_t i = 0; i < entities[1].ammo; i++) entities[i+2+OBSTACLES_QTY+entities[0].ammo] = ENTITY_BULLET_OF_CPU;
}

int main(){
    InitWindow(SCREEN_DIMENSIONS.x, SCREEN_DIMENSIONS.y, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    int entities_qty = ENTITY_PLAYER.ammo + ENTITY_CPU.ammo + OBSTACLES_QTY + 2;
    Entity entities[entities_qty];
    restart_game(entities);
    Entity *player = &entities[0];
    Entity *cpu = &entities[1];
    normalize(&(*cpu).direction);
    
    while (!WindowShouldClose()) {
        //INPUT
        (*player).direction = (Vector2){0,0};
        if(IsKeyDown(KEY_UP)) (*player).direction.y = -1;
        if(IsKeyDown(KEY_DOWN)) (*player).direction.y = 1;
        if(IsKeyDown(KEY_RIGHT)) (*player).direction.x = 1;
        if(IsKeyDown(KEY_LEFT)) (*player).direction.x = -1;
        if(IsKeyPressed(KEY_R)) restart_game(entities);
        if(IsKeyPressed(KEY_SPACE)) spawn_bullet((*player), entities, entities_qty);
        normalize(&(*player).direction);

        //UPDATE
        auto_spawn_bullet(*cpu, entities, entities_qty);
        for (uint8_t i = 0; i < entities_qty; i++) update_entity(&entities[i]);

        //DRAW
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        for (uint8_t i = 0; i < entities_qty; i++) draw_entity(entities[i]);
        
        //DEBUG
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}