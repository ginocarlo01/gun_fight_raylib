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

const Vector2 AIM_DIRECTIONS[3] = {
    {1, 0},   // horizontal
    {1, -1},  // 45° para cima
    {1, 1}    // 45° para baixo
};

typedef struct Entity Entity;

typedef struct Entity {
    EntityType type;
    Vector2 position;
    Vector2 direction;
    float radius;
    float current_speed;
    Color color;
    int damage;
    int max_ammo;
    int ammo;            // novo campo para balas do player
    Entity *bullets;     // ponteiro para array de bullets
    bool enabled;
    int aim_index;
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
Entity create_entity(EntityType type, Vector2 direction ,Vector2 position, Color color, float radius, float speed, int damage, int ammo, bool enabled) {
    Entity e = {0};
    e.type = type;
    e.position = position;
    e.direction = direction;
    e.radius = radius;
    e.current_speed = speed;
    e.color = color;
    e.damage = damage;
    e.ammo = ammo;
    e.max_ammo = ammo;
    e.enabled = enabled;
    e.aim_index = 0;
    return e;
}

void spawn_bullet(Entity *player) {
    if (player->ammo <= 0) return;

    Vector2 dir = AIM_DIRECTIONS[player->aim_index];
    normalize(&dir);

    float line_len = 30.0f;

    int index = player->max_ammo - player->ammo;
    player->bullets[index].enabled = true;
    player->bullets[index].position.x = player->position.x + dir.x * (player->radius + line_len);
    player->bullets[index].position.y = player->position.y + dir.y * (player->radius + line_len);

    player->bullets[index].direction = AIM_DIRECTIONS[player->aim_index];
    
    normalize(&player->bullets[index].direction);

    player->ammo--;
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
        if (entity->position.y - entity->radius < 0 || entity->position.y - entity->radius > GetScreenHeight()){
            entity->direction.y *= -1;
        }
    } else if (entity->type == ENTITY_PLAYER) {
        if (entity->position.y < 0) entity->position.y = 0;
        if (entity->position.y > GetScreenHeight()) entity->position.y = GetScreenHeight();
        if (entity->position.x < 0) entity->position.x = 0;
        if (entity->position.x > GetScreenWidth()) entity->position.x = GetScreenWidth();
        float playerLeft  = 0.05f * GetScreenWidth();  // 5% da tela
        float playerRight = 0.3f  * GetScreenWidth();  // 30% da tela
        if (entity->position.x < playerLeft)  entity->position.x = playerLeft;
        if (entity->position.x > playerRight) entity->position.x = playerRight;
    } else if (entity->type == ENTITY_OBSTACLE) {
        if (entity->position.y < 0){
            entity->position.y = 0;
            entity->direction.y *= -1;
        }
            
        if (entity->position.y > GetScreenHeight()) {
            entity->position.y = GetScreenHeight();
            entity->direction.y *= -1;
        }
        
    }

    
}

    


// Desenha entidade
void draw_entity(Entity *e) {
    if (!e->enabled) return;

    // Desenha a própria entity
    DrawCircleV(e->position, e->radius, e->color);

    if (e->type == ENTITY_PLAYER) {
        float line_len = 40; // tamanho do retângulo de mira
        for (int i = 0; i < 3; i++) {
            Color col = (i == e->aim_index) ? YELLOW : GRAY;
            Vector2 end = { e->position.x + AIM_DIRECTIONS[i].x * line_len,
                            e->position.y + AIM_DIRECTIONS[i].y * line_len };
            DrawLineV(e->position, end, col);
            }
    }


    // Se tiver bullets, desenha cada uma que estiver ativa
    if (e->bullets != NULL) {
        int bullets_fired = e->max_ammo - e->ammo;
        for (int i = 0; i < bullets_fired; i++) {
            if (e->bullets[i].enabled) {
                DrawCircleV(e->bullets[i].position, e->bullets[i].radius, e->bullets[i].color);
            }
        }
    }
}

void bullet_check_collision(Entity *bullet, Entity *entity) {
    
    if(entity->type == ENTITY_BULLET || !entity->enabled ){return;}
    
    if (CheckCollisionCircles(bullet->position, bullet->radius, entity->position, entity->radius)) {
        if(entity->type == ENTITY_OBSTACLE){entity->enabled = false;}
        bullet->enabled = false;
        
        //TODO add sound
        //play_ball_hit();
    }
}

int main(void) {
    const int screenWidth = 960;
    const int screenHeight = 540;
    InitWindow(screenWidth, screenHeight, "Gun Fight");
    SetTargetFPS(60);

    Entity player_bullets[5]; // array externo
    Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){70, screenHeight/2}, BLUE, 20, 300, 0, 5, true);
    player.ammo = 5;
    player.bullets = player_bullets;
    for (int i = 0; i < player.ammo; i++) {
        player_bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, RED, 10, 400, 1, 0, false);
    }

    int num_obstacles = 4;
    Entity obstacles[num_obstacles];
    float startX = 0.3f * screenWidth;
    float endX   = 0.7f * screenWidth;
    float spacing = (endX - startX) / (num_obstacles + 1);

    for (int i = 0; i < num_obstacles; i++) {
        float x = startX + spacing * (i + 1);
        float y = screenHeight / 2;
        obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,y}, WHITE, 20, 300, 0, 0, true);
    }

    
    Entity bullets[5];
    for (int i = 0; i < 5; i++) {
        bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, RED, 10, 400, 1 , 0,false);
    }


    while (!WindowShouldClose()) {
        // INPUT
        player.direction = (Vector2){0,0};
        if (IsKeyDown(KEY_UP)) player.direction.y = -1;
        if (IsKeyDown(KEY_DOWN)) player.direction.y = 1;
        if (IsKeyDown(KEY_LEFT)) player.direction.x = -1;
        if (IsKeyDown(KEY_RIGHT)) player.direction.x = 1;
        normalize(&player.direction);

        if (IsKeyPressed(KEY_SPACE)) spawn_bullet(&player);

        // Alterna mira
        if (IsKeyPressed(KEY_W)) {
            player.aim_index++;
            if (player.aim_index > 2) player.aim_index = 0; // ciclo para baixo
            
        }
        if (IsKeyPressed(KEY_S)) {
            player.aim_index--;
            if (player.aim_index < 0) player.aim_index = 2; // ciclo para cima
        }


        // LOGIC
        update_entity(&player);
        for(int i = 0; i < num_obstacles; i++) {
            update_entity(&obstacles[i]);
        }
        
        for(int i = 0; i < player.max_ammo - player.ammo; i++) {
        if (player.bullets[i].enabled) {
            update_entity(&player.bullets[i]);
            // checa colisão com todos os obstáculos
            for(int j = 0; j < num_obstacles; j++) {
                bullet_check_collision(&player.bullets[i], &obstacles[j]);
            }
        }
    }

            


        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);
        draw_entity(&player);
        draw_entity(&player);
        for(int i = 0; i < num_obstacles; i++) {
            draw_entity(&obstacles[i]);
        }
        for (int i = 0; i < 5; i++) draw_entity(&bullets[i]);

        // FOR DEBUG 
        //DrawLine(70, 0, 2 + 70, screenHeight, BLUE); //player start 
        DrawLine(70 - 20, 0, 2 + 70 - 20, screenHeight, BLUE); //player limit left 
        DrawLine(70 - 20 + 240, 0, 2 + 70 - 20 + 240, screenHeight, BLUE); //player limit right 
        DrawLine(70 - 20 + 240 + 60, 0, 2 + 70 - 20 + 240 + 60, screenHeight, WHITE); //obstacle track 1

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
