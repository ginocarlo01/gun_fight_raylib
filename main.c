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
    int life;
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
Entity create_entity(EntityType type, Vector2 direction ,Vector2 position, Color color, float radius, float speed, int damage, int ammo, int life, bool enabled) {
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
    e.life = life;
    e.enabled = enabled;
    e.aim_index = 0;
    return e;
}

void spawn_bullet(Entity *entity) {
    if (entity->ammo <= 0) return;

    Vector2 dir = AIM_DIRECTIONS[entity->aim_index];
    normalize(&dir);

    float line_len = 30.0f;

    int index = entity->max_ammo - entity->ammo;
    entity->bullets[index].enabled = true;
    entity->bullets[index].position.x = entity->position.x + dir.x * (entity->radius + line_len);
    entity->bullets[index].position.y = entity->position.y + dir.y * (entity->radius + line_len);

    Vector2 direction = AIM_DIRECTIONS[entity->aim_index];

    if(entity->type == ENTITY_CPU){
        direction.x *= -1;
    }

    entity->bullets[index].direction = direction;
    
    normalize(&entity->bullets[index].direction);

    entity->ammo--;
}

void update_bullet_check(Entity *entity, float *current_timer, float max_timer){
    *current_timer += GetFrameTime();
    if(*current_timer >= max_timer){
        *current_timer = 0;

        //index
        entity->aim_index--;
        if (entity->aim_index < 0) entity->aim_index = 2;

        //spawn
        spawn_bullet(entity);
    }
}


// Atualiza entidade
void update_entity(Entity *entity) {
    if (!entity->enabled) return;
    float delta = GetFrameTime();
    entity->position.x += entity->current_speed * entity->direction.x * delta;
    entity->position.y += entity->current_speed * entity->direction.y * delta;

    //TODO trocar pra switch 
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

        float limitLeft  = 0.05f * GetScreenWidth();          float limitRight = 0.3f  * GetScreenWidth();  
        if (entity->position.x < limitLeft)  entity->position.x = limitLeft;
        if (entity->position.x > limitRight) entity->position.x = limitRight;
    } else if (entity->type == ENTITY_CPU) {
        if (entity->position.y < 0){
            entity->position.y = 0;
            entity->direction.y *= -1;
        }
        
        if (entity->position.y > GetScreenHeight()){
            entity->position.y = GetScreenHeight();
            entity->direction.y *= -1;
        }
        

        float limitLeft  = 0.7f * GetScreenWidth(); 
        float limitRight = 0.95f  * GetScreenWidth();  
        if (entity->position.x < limitLeft)  entity->position.x = limitLeft;
        if (entity->position.x > limitRight) entity->position.x = limitRight;
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
    if (e->type == ENTITY_CPU) {
        float line_len = 40; // tamanho do retângulo de mira
        for (int i = 0; i < 3; i++) {
            Color col = (i == e->aim_index) ? YELLOW : GRAY;
            Vector2 end = { e->position.x + AIM_DIRECTIONS[i].x * -1 * line_len,
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

void handle_entity_life(Entity *entity, int damage){
    entity->life -= damage;
    if(entity->life <= 0){
        entity->enabled = false;
        //TODO
        //restart game loop
    }
}


void bullet_check_collision(Entity *bullet, Entity *hit_entity, Entity *parent) {
    
    if(!hit_entity->enabled ){return;}
    
    if (CheckCollisionCircles(bullet->position, bullet->radius, hit_entity->position, hit_entity->radius)) {
        if(hit_entity->type == ENTITY_OBSTACLE){
            hit_entity->enabled = false;
            parent->life += (-1) * bullet->damage;
        }
        else if(hit_entity->type == ENTITY_PLAYER || hit_entity->type == ENTITY_CPU) {
            handle_entity_life(hit_entity, bullet->damage);
        }
            
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
    Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){70, screenHeight/2}, BLUE, 20, 300, 0, 5, 1,true);
    player.ammo = 5;
    //TODO checar se eh possivel otimizar
    player.bullets = player_bullets;
    for (int i = 0; i < player.ammo; i++) {
        player.bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, RED, 10, 400, 1, 0, 0, false);
    }

    //CPU
    //TODO verificar se eh possivel colocar em uma funcao
    Entity cpu_bullets[5]; 
    Entity cpu = create_entity(ENTITY_CPU, (Vector2){0,1}, (Vector2){screenWidth - 70, screenHeight/2}, RED, 20, 200, 0, 5, 1,true);
    cpu.ammo = 5;
    //TODO checar se eh possivel otimizar
    cpu.bullets = cpu_bullets;
    for (int i = 0; i < cpu.ammo; i++) {
        cpu.bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, RED, 10, 400, 1, 0, 0, false);
    }

    int num_obstacles = 4;
    Entity obstacles[num_obstacles];
    float startX = 0.3f * screenWidth;
    float endX   = 0.7f * screenWidth;
    float spacing = (endX - startX) / (num_obstacles + 1);

    for (int i = 0; i < num_obstacles; i++) {
        float x = startX + spacing * (i + 1);
        float y = screenHeight / 2;
        if(i == 1){
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,y}, RED, 20, 400, 1, 0, 0, true);
        }
        else if(i == 2){
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,y}, GREEN, 20, 250, -1, 0, 0, true);
        }
        else {
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,y}, WHITE, 20, 200, 1, 0, 0, true);
        }
    }

    // TODO colocar no config superior
    float current_cpu_timer = 0.0;
    float max_cpu_timer = 3.0;


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
        update_entity(&cpu);
        update_bullet_check(&cpu, &current_cpu_timer, max_cpu_timer);

        for(int i = 0; i < num_obstacles; i++) {
            update_entity(&obstacles[i]);
        }
        
        for(int i = 0; i < player.max_ammo - player.ammo; i++) {
            if (player.bullets[i].enabled) {
                update_entity(&player.bullets[i]);
                // checa colisão com todos os obstáculos
                for(int j = 0; j < num_obstacles; j++) {
                    bullet_check_collision(&player.bullets[i], &obstacles[j], &player);
                    bullet_check_collision(&player.bullets[i], &cpu, &player);
                }
        }
        }

        for(int i = 0; i < cpu.max_ammo - cpu.ammo; i++) {
            if (cpu.bullets[i].enabled) {
                update_entity(&cpu.bullets[i]);
                // checa colisão com todos os obstáculos
                for(int j = 0; j < num_obstacles; j++) {
                    bullet_check_collision(&cpu.bullets[i], &obstacles[j], &cpu);
                    bullet_check_collision(&cpu.bullets[i], &player, &cpu);
                }
        }
        }

        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);
        draw_entity(&player);
        draw_entity(&cpu);
        for(int i = 0; i < num_obstacles; i++) {
            draw_entity(&obstacles[i]);
        }
        // for (int i = 0; i < 5; i++) draw_entity(&bullets[i]);

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
