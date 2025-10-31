#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef enum {
    ENTITY_PLAYER,
    ENTITY_BULLET,
    ENTITY_OBSTACLE,
    ENTITY_CPU
} EntityType;

typedef struct Entity {
    EntityType type;
    Vector2 position;
    Vector2 direction;
    float radius;
    float current_speed;
    Color color;
    int damage;
    bool enabled;
} Entity;

// Normaliza vetor
void normalize(Vector2 *v) {
    float mag = sqrtf(v->x * v->x + v->y * v->y);
    if (mag != 0.0f) {
        v->x /= mag;
        v->y /= mag;
    }
}

// Cria entidade
Entity create_entity(EntityType type, Vector2 direction ,Vector2 position, Color color, float radius, float speed, int damage, bool enabled) {
    Entity e = {0};
    e.type = type;
    e.position = position;
    e.direction = direction;
    e.radius = radius;
    e.current_speed = speed;
    e.color = color;
    damage = 0;
    e.enabled = enabled;
    return e;
}

// Ativa uma bala do player
void spawn_bullet(Entity player, Entity bullets[5]) {
    for (int i = 0; i < 5; i++) {
        if (!bullets[i].enabled) {
            bullets[i].enabled = true;
            bullets[i].position = player.position;
            bullets[i].direction = (Vector2){1, 0}; // move para a direita
            break;
        }
    }
}

// Atualiza entidade
void update_entity(Entity *entity) {
    if (!entity->enabled) return;
    float delta = GetFrameTime();
    entity->position.x += entity->current_speed * entity->direction.x * delta;
    entity->position.y += entity->current_speed * entity->direction.y * delta;

    if (entity->type == ENTITY_BULLET) {
        if (entity->position.x - entity->radius > GetScreenWidth() || entity->position.x - entity->radius < 0) {
            entity->enabled = false;
        }
    } else if (entity->type == ENTITY_PLAYER) {
        if (entity->position.y < 0) entity->position.y = 0;
        if (entity->position.y > GetScreenHeight()) entity->position.y = GetScreenHeight();
        if (entity->position.x < 0) entity->position.x = 0;
        if (entity->position.x > GetScreenWidth()) entity->position.x = GetScreenWidth();
    } else if (entity->type == ENTITY_OBSTACLE) {
        if (entity->position.y < 0){
            entity->position.y = 0;
            entity->direction.y = 1;
        }
            
        if (entity->position.y > GetScreenHeight()) {
            entity->position.y = GetScreenHeight();
            entity->direction.y = 1;
        }
        
    }
    
}

// Desenha entidade
void draw_entity(Entity *e) {
    if (!e->enabled) return;
    DrawCircleV(e->position, e->radius, e->color);
}

int main(void) {
    const int screenWidth = 960;
    const int screenHeight = 540;
    InitWindow(screenWidth, screenHeight, "Gun Fight");
    SetTargetFPS(60);

    Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){70,screenHeight/2}, BLUE, 20, 300, 0, true);
    Entity obstacle = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){70 - 20 + 240 + 60,screenHeight/2},  WHITE, 20, 300, 0, true);
    Entity bullets[5];
    for (int i = 0; i < 5; i++) {
        bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, RED, 10, 400, 1 ,false);
    }

    while (!WindowShouldClose()) {
        // INPUT
        player.direction = (Vector2){0,0};
        if (IsKeyDown(KEY_UP)) player.direction.y = -1;
        if (IsKeyDown(KEY_DOWN)) player.direction.y = 1;
        if (IsKeyDown(KEY_LEFT)) player.direction.x = -1;
        if (IsKeyDown(KEY_RIGHT)) player.direction.x = 1;
        normalize(&player.direction);

        if (IsKeyPressed(KEY_SPACE)) spawn_bullet(player, bullets);

        // UPDATE
        update_entity(&player);
        update_entity(&obstacle);
        for (int i = 0; i < 5; i++) update_entity(&bullets[i]);

        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);
        draw_entity(&player);
        draw_entity(&obstacle);
        for (int i = 0; i < 5; i++) draw_entity(&bullets[i]);

        // FOR DEBUG 
        DrawLine(70, 0, 2 + 70, screenWidth, BLUE); //player start 
        DrawLine(70 - 20, 0, 2 + 70 - 20, screenWidth, BLUE); //player limit left 
        DrawLine(70 - 20 + 240, 0, 2 + 70 - 20 + 240, screenWidth, BLUE); //player limit right 
        DrawLine(70 - 20 + 240 + 60, 0, 2 + 70 - 20 + 240 + 60, screenWidth, WHITE); //obstacle track 1

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
