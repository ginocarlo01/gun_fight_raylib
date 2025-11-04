#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*==============================================================================
    CONFIG
==============================================================================*/

// Game settings
static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;
static const char * GAME_TITLE = "Gun Fight";
static const int TARGET_FPS = 240;

// Movement constraints
static const float LIMIT_PLAYER_LEFT = 0.05f;
static const float LIMIT_PLAYER_RIGHT = 0.30f;
static const float LIMIT_OBSTACLE_LEFT = 0.30f;
static const float LIMIT_OBSTACLE_RIGHT = 0.70f;
static const float LIMIT_CPU_LEFT = 0.7f;
static const float LIMIT_CPU_RIGHT = 0.95f;

// Aiming settings
static const Vector2 AIM_DIRECTIONS[3] = {
    {1, 0},
    {1, -1},
    {1, 1}  
};
static const int INITIAL_AIM_INDEX = 0;
static const int LENGTH_LINE_AIM = 40;
static const size_t AIM_DIRECTIONS_LEN = sizeof(AIM_DIRECTIONS) / sizeof(AIM_DIRECTIONS[0]);

// Player settings
static const float PLAYER_START_POSITION_X = LIMIT_PLAYER_LEFT * 5 * SCREEN_WIDTH;
static const float PLAYER_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color PLAYER_COLOR = ORANGE;
static const int PLAYER_RADIUS = 20;
static const int PLAYER_SPEED = 300;
static const int PLAYER_DAMAGE = 0;
static const int PLAYER_AMMO = 15;
static const int PLAYER_LIFE = 3;

// Player bullet settings
static const Color PLAYER_BULLET_COLOR = GREEN;
static const int PLAYER_BULLET_RADIUS = 10;
static const int PLAYER_BULLET_SPEED = 400;
static const int PLAYER_BULLET_DAMAGE = 1;

// CPU settings
static const float CPU_START_POSITION_X = SCREEN_WIDTH * 0.85f;
static const float CPU_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color CPU_COLOR = RED;
static const int CPU_RADIUS = 20;
static const int CPU_SPEED = 200;
static const int CPU_DAMAGE = 0;
static const int CPU_AMMO = 15;
static const int CPU_LIFE = 1;

// CPU bullet settings
static const Color CPU_BULLET_COLOR = RED;
static const int CPU_BULLET_RADIUS = 10;
static const int CPU_BULLET_SPEED = 400;
static const int CPU_BULLET_DAMAGE = 1;

static const float MAX_CPU_TIMER = 3.0f;

static const int QTY_OBSTACLES = 4;

// Default obstacle
static const float OBSTACLE_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color OBSTACLE_COLOR = WHITE;
static const int OBSTACLE_RADIUS = 20;
static const int OBSTACLE_SPEED = 200;
static const int OBSTACLE_DAMAGE = 0;

// Damaging obstacle
static const Color OBSTACLE_DAMAGE_COLOR = RED;
static const int OBSTACLE_DAMAGE_RADIUS = 20;
static const int OBSTACLE_DAMAGE_SPEED = 250;
static const int OBSTACLE_DAMAGE_DAMAGE = 1;

// Healing obstacle
static const Color OBSTACLE_HEALING_COLOR = GREEN;
static const int OBSTACLE_HEALING_RADIUS = 20;
static const int OBSTACLE_HEALING_SPEED = 200;
static const int OBSTACLE_HEALING_DAMAGE = -1;

// Entity definitions
typedef enum {
    ENTITY_PLAYER,
    ENTITY_BULLET,
    ENTITY_OBSTACLE,
    ENTITY_CPU
} EntityType;


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
    int ammo;            
    Entity *bullets;     
    int life;
    bool enabled;
    int aim_index;
} Entity;

static Sound ball_hit_sfx;
static Sound player_win_sfx;
static Sound player_lose_sfx;

static const char* ball_hit_sfx_path = "sfx/ball_hit.wav";
static const char* player_win_sfx_path = "sfx/win.wav";
static const char* player_lose_sfx_path = "sfx/lose.wav";

/*==============================================================================
    FUNCTIONS   
==============================================================================*/
void normalize(Vector2 *v) {
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
    e.aim_index = INITIAL_AIM_INDEX;
    
    // Create bullets array if ammo > 1
    if (ammo > 1) {
        e.bullets = malloc(sizeof(Entity) * ammo);
        for (int i = 0; i < ammo; i++) {
            e.bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, 
                                       type == ENTITY_PLAYER ? PLAYER_BULLET_COLOR : CPU_BULLET_COLOR,
                                       type == ENTITY_PLAYER ? PLAYER_BULLET_RADIUS : CPU_BULLET_RADIUS,
                                       type == ENTITY_PLAYER ? PLAYER_BULLET_SPEED : CPU_BULLET_SPEED,
                                       type == ENTITY_PLAYER ? PLAYER_BULLET_DAMAGE : CPU_BULLET_DAMAGE,
                                       0, 0, false);
        }
    }
    
    return e;
}

void spawn_bullet(Entity *entity) {
    if (entity->ammo <= 0) return;

    Vector2 dir = AIM_DIRECTIONS[entity->aim_index];

    int index = entity->max_ammo - entity->ammo;
    entity->bullets[index].enabled = true;
    entity->bullets[index].position.x = entity->position.x + dir.x * (entity->radius + LENGTH_LINE_AIM);
    entity->bullets[index].position.y = entity->position.y + dir.y * (entity->radius + LENGTH_LINE_AIM);

    
    dir.x *= (entity->type == ENTITY_CPU) ? -1.0f : 1.0f;
    normalize(&dir);    
    entity->bullets[index].direction = dir;

    entity->ammo--;

    PlaySound(ball_hit_sfx);
}

void update_bullet_check(Entity *entity, float *current_timer, float max_timer){
    *current_timer += GetFrameTime();
    if(*current_timer >= max_timer){
        *current_timer = 0;

        entity->aim_index--;
        if (entity->aim_index < 0) entity->aim_index = AIM_DIRECTIONS_LEN - 1;

        spawn_bullet(entity);
        
    }
}


void update_entity(Entity *entity) {
    if (!entity->enabled) return;
    float delta = GetFrameTime();
    entity->position.x += entity->current_speed * entity->direction.x * delta;
    entity->position.y += entity->current_speed * entity->direction.y * delta;
    switch (entity->type) {
        case ENTITY_BULLET:
            if (entity->position.x - entity->radius > SCREEN_WIDTH || entity->position.x - entity->radius < 0) {
                entity->enabled = false;
                PlaySound(ball_hit_sfx);
            }
            if (entity->position.y - entity->radius < 0 || entity->position.y - entity->radius > SCREEN_HEIGHT) {
                entity->direction.y *= -1;
                PlaySound(ball_hit_sfx);
            }
            break;

        case ENTITY_PLAYER:
            if (entity->position.y < 0) entity->position.y = 0;
            if (entity->position.y > SCREEN_HEIGHT) entity->position.y = SCREEN_HEIGHT;

            float limitLeft = LIMIT_PLAYER_LEFT * SCREEN_WIDTH;
            float limitRight = LIMIT_PLAYER_RIGHT * SCREEN_WIDTH;
            if (entity->position.x < limitLeft) entity->position.x = limitLeft;
            if (entity->position.x > limitRight) entity->position.x = limitRight;
            break;

        case ENTITY_CPU:
            float cpuLimitLeft = LIMIT_CPU_LEFT * SCREEN_WIDTH;
            float cpuLimitRight = LIMIT_CPU_RIGHT * SCREEN_WIDTH;

            if (entity->position.y < 0) {
                entity->position.y = 0;
                entity->direction.y *= -1;
            }
            if (entity->position.y > SCREEN_HEIGHT) {
                entity->position.y = SCREEN_HEIGHT;
                entity->direction.y *= -1;
            }
            if (entity->position.x < cpuLimitLeft){
                entity->position.x = cpuLimitLeft;
                entity->direction.x *= -1;
            }
            if (entity->position.x > cpuLimitRight){
                entity->position.x = cpuLimitRight;
                entity->direction.x *= -1;
            }
            
            break;

        case ENTITY_OBSTACLE:
            if (entity->position.y < 0) {
                entity->position.y = 0;
                entity->direction.y *= -1;
                PlaySound(ball_hit_sfx);
            }
            if (entity->position.y > SCREEN_HEIGHT) {
                entity->position.y = SCREEN_HEIGHT;
                entity->direction.y *= -1;
                PlaySound(ball_hit_sfx);
            }
            break;
    }
}


void draw_entity(Entity *e) {
    if (!e->enabled) return;

    DrawCircleV(e->position, e->radius, e->color);

    if (e->type == ENTITY_PLAYER || e->type == ENTITY_CPU) {
        float line_len = LENGTH_LINE_AIM;
        float mx = (e->type == ENTITY_CPU) ? -1.0f : 1.0f; 
        for (int i = 0; i < AIM_DIRECTIONS_LEN; i++) {
            Color col = (i == e->aim_index) ? YELLOW : GRAY;
            Vector2 dir = AIM_DIRECTIONS[i];
            Vector2 end = { e->position.x + dir.x * mx * line_len,
                            e->position.y + dir.y * line_len };
            DrawLineV(e->position, end, col);
        }
    }

    // Draw bullets
    if (e->bullets != NULL) {
        int bullets_fired = e->max_ammo - e->ammo;
        for (int i = 0; i < bullets_fired; i++) {
            if (e->bullets[i].enabled) {
                DrawCircleV(e->bullets[i].position, e->bullets[i].radius, e->bullets[i].color);
            }
        }
    }
}

void handle_entity_life(Entity *entity, int damage, bool *restart_flag, int *player_win_count, int *cpu_win_count) {
    entity->life -= damage;
    if (entity->life <= 0) {
        entity->enabled = false;
        *restart_flag = true;

        // Atualiza contador de vitória do oponente
        if (entity->type == ENTITY_PLAYER) {
            (*cpu_win_count)++;
            PlaySound(player_lose_sfx);
        } else if (entity->type == ENTITY_CPU) {
            (*player_win_count)++;
            PlaySound(player_win_sfx);
        }
    }
}



void bullet_check_collision(Entity *bullet, Entity *hit_entity, Entity *parent, bool *restart_flag, int *player_win_count, int *cpu_win_count) {
    
    if(!hit_entity->enabled ){return;}
    
    if (CheckCollisionCircles(bullet->position, bullet->radius, hit_entity->position, hit_entity->radius)) {
        if(hit_entity->type == ENTITY_OBSTACLE){
            hit_entity->enabled = false;
            parent->life += (-1) * hit_entity->damage;
        }
        else if(hit_entity->type == ENTITY_PLAYER || hit_entity->type == ENTITY_CPU) {
            handle_entity_life(hit_entity, bullet->damage, restart_flag, player_win_count, cpu_win_count);
        }
            
        bullet->enabled = false;
        
        //TODO add sound
        //play_ball_hit();
    }
}
void cleanup_entities(Entity *player, Entity *cpu) {
    if (player != NULL) {
        if (player->bullets != NULL) {
            free(player->bullets);
            player->bullets = NULL;
        }
        
    }

    if (cpu != NULL) {
        if (cpu->bullets != NULL) {
            free(cpu->bullets);
            cpu->bullets = NULL;
        }
        
    }
}

void reset_entity(Entity *e) {
    e->enabled = true;
    
    e->ammo = e->max_ammo;
    e->aim_index = INITIAL_AIM_INDEX;

    switch (e->type) {
        case ENTITY_PLAYER:
            e->position = (Vector2){PLAYER_START_POSITION_X, PLAYER_START_POSITION_Y};
            e->life = PLAYER_LIFE;
            e->direction = (Vector2){0,0};
            break;
        case ENTITY_CPU:
            e->position = (Vector2){CPU_START_POSITION_X, CPU_START_POSITION_Y};
            e->life = CPU_LIFE;
            e->direction = (Vector2){1,1};
            break;
        case ENTITY_OBSTACLE:
            e->enabled = true;
            e->direction = (Vector2){0, 1};
            e->position.y = OBSTACLE_START_POSITION_Y;
            break;
        default:
            break;
    }

    if (e->bullets != NULL) {
        for (int i = 0; i < e->max_ammo; i++) {
            e->bullets[i].enabled = false;
            e->bullets[i].position = (Vector2){0,0};
            e->bullets[i].direction = (Vector2){0,0};
        }
    }
}

void reset_game_state(Entity *player, Entity *cpu, Entity obstacles[], float *cpu_timer) {
    reset_entity(player);
    reset_entity(cpu);

    for (int i = 0; i < QTY_OBSTACLES; i++) {
        reset_entity(&obstacles[i]);
    }

    *cpu_timer = 0.0f;
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    audio_init();
    /*==============================================================================
            INITIALIZATION
    ==============================================================================*/

    Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){PLAYER_START_POSITION_X, PLAYER_START_POSITION_Y}, PLAYER_COLOR, PLAYER_RADIUS, PLAYER_SPEED, PLAYER_DAMAGE, PLAYER_AMMO, PLAYER_LIFE,true);
    Entity cpu = create_entity(ENTITY_CPU, (Vector2){1,1}, (Vector2){CPU_START_POSITION_X, CPU_START_POSITION_Y}, CPU_COLOR, CPU_RADIUS, CPU_SPEED, CPU_DAMAGE, CPU_AMMO, CPU_LIFE, true);
    
    Entity obstacles[QTY_OBSTACLES];
    float startX = LIMIT_OBSTACLE_LEFT * SCREEN_WIDTH;
    float endX   = LIMIT_OBSTACLE_RIGHT * SCREEN_WIDTH;
    float spacing = (endX - startX) / (QTY_OBSTACLES + 1);

    for (int i = 0; i < QTY_OBSTACLES; i++) {
        float x = startX + spacing * (i + 1);
        if(i == 1){
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,OBSTACLE_START_POSITION_Y}, OBSTACLE_DAMAGE_COLOR, OBSTACLE_DAMAGE_RADIUS, OBSTACLE_DAMAGE_SPEED, OBSTACLE_DAMAGE_DAMAGE, 0, 0, true);
        }
        else if(i == 2){
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x,OBSTACLE_START_POSITION_Y}, OBSTACLE_HEALING_COLOR, OBSTACLE_HEALING_RADIUS, OBSTACLE_HEALING_SPEED, OBSTACLE_HEALING_DAMAGE, 0, 0, true);
        }
        else {
            obstacles[i] = create_entity(ENTITY_OBSTACLE, (Vector2){0,1}, (Vector2){x, OBSTACLE_START_POSITION_Y}, OBSTACLE_COLOR, OBSTACLE_RADIUS, OBSTACLE_SPEED, OBSTACLE_DAMAGE, 0, 0, true);
        }
    }

    float current_cpu_timer = 0.0;
    int player_win_count = 0;
    int cpu_win_count = 0;
    bool restart = false;


    while (!WindowShouldClose()) {
        
        /*==============================================================================
            INPUT
        ==============================================================================*/

        player.direction = (Vector2){0,0};
        if (IsKeyDown(KEY_UP)) player.direction.y = -1;
        if (IsKeyDown(KEY_DOWN)) player.direction.y = 1;
        if (IsKeyDown(KEY_LEFT)) player.direction.x = -1;
        if (IsKeyDown(KEY_RIGHT)) player.direction.x = 1;
        normalize(&player.direction);
        normalize(&cpu.direction);

        if (IsKeyPressed(KEY_SPACE)) spawn_bullet(&player);

        if (IsKeyPressed(KEY_W)) {
            player.aim_index++;
            if (player.aim_index > AIM_DIRECTIONS_LEN-1) player.aim_index = 0; // ciclo para baixo
            
        }
        if (IsKeyPressed(KEY_S)) {
            player.aim_index--;
            if (player.aim_index < 0) player.aim_index = AIM_DIRECTIONS_LEN-1; // ciclo para cima
        }


        /*==============================================================================
            LOGIC
        ==============================================================================*/

        update_entity(&player);
        update_entity(&cpu);
        update_bullet_check(&cpu, &current_cpu_timer, MAX_CPU_TIMER);

        for(int i = 0; i < QTY_OBSTACLES; i++) {
            update_entity(&obstacles[i]);
        }

        // Check collisions
        
        for(int i = 0; i < player.max_ammo - player.ammo; i++) {
            if (player.bullets[i].enabled) {
                update_entity(&player.bullets[i]);
                for(int j = 0; j < QTY_OBSTACLES; j++) {
                    bullet_check_collision(&player.bullets[i], &obstacles[j], &player, &restart, &player_win_count, &cpu_win_count);
                    bullet_check_collision(&player.bullets[i], &cpu, &player, &restart, &player_win_count, &cpu_win_count);
                }
        }
        }

        for(int i = 0; i < cpu.max_ammo - cpu.ammo; i++) {
            if (cpu.bullets[i].enabled) {
                update_entity(&cpu.bullets[i]);
                for(int j = 0; j < QTY_OBSTACLES; j++) {
                    bullet_check_collision(&cpu.bullets[i], &obstacles[j], &cpu, &restart, &player_win_count, &cpu_win_count);
                    bullet_check_collision(&cpu.bullets[i], &player, &cpu, &restart, &player_win_count, &cpu_win_count);
                }
        }
        }

        if(restart){
        DrawText("Restarting...", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 30, WHITE);
        EndDrawing();

        WaitTime(1.0);  // pausa opcional pra mostrar feedback
        reset_game_state(&player, &cpu, obstacles, &current_cpu_timer);
        restart = false;
        continue;
        }

        /*==============================================================================
            DRAW
        ==============================================================================*/

        BeginDrawing();
        ClearBackground(BLACK);

        draw_entity(&player);
        draw_entity(&cpu);

        for(int i = 0; i < QTY_OBSTACLES; i++) {
            draw_entity(&obstacles[i]);
        }

        /*==============================================================================
            DEBUG LINES
        ==============================================================================*/

        DrawLine(LIMIT_PLAYER_LEFT * SCREEN_WIDTH, 0, LIMIT_PLAYER_LEFT * SCREEN_WIDTH, SCREEN_HEIGHT, BLUE); 
        DrawLine(LIMIT_PLAYER_RIGHT * SCREEN_WIDTH, 0, LIMIT_PLAYER_RIGHT * SCREEN_WIDTH, SCREEN_HEIGHT, BLUE); 
        DrawLine(LIMIT_OBSTACLE_LEFT * SCREEN_WIDTH, 0, LIMIT_OBSTACLE_LEFT * SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); 
        DrawLine(LIMIT_OBSTACLE_RIGHT * SCREEN_WIDTH, 0, LIMIT_OBSTACLE_RIGHT * SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); 
        DrawLine(LIMIT_CPU_LEFT * SCREEN_WIDTH, 0, LIMIT_CPU_LEFT * SCREEN_WIDTH, SCREEN_HEIGHT, RED); 
        DrawLine(LIMIT_CPU_RIGHT * SCREEN_WIDTH, 0, LIMIT_CPU_RIGHT * SCREEN_WIDTH, SCREEN_HEIGHT, RED); 
        
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);
        DrawText(TextFormat("Player Life: %i", player.life), 10, 55, 20, PLAYER_COLOR);
        DrawText(TextFormat("Player Ammo: %i/%i", player.ammo, player.max_ammo), 10, 80, 20, PLAYER_COLOR);
        DrawText(TextFormat("Player Wins: %i", player_win_count), 10, 105, 20, PLAYER_COLOR);

        DrawText(TextFormat("CPU Life: %i", cpu.life), SCREEN_WIDTH - 260, 30, 20, CPU_COLOR);
        DrawText(TextFormat("CPU Ammo: %i/%i", cpu.ammo, cpu.max_ammo), SCREEN_WIDTH - 260, 55, 20, CPU_COLOR);
        DrawText(TextFormat("CPU Wins: %i", cpu_win_count), SCREEN_WIDTH - 260, 80, 20, CPU_COLOR);

        EndDrawing();
    }

    /*==============================================================================
            CLEANUP
    ==============================================================================*/
    
    cleanup_entities(&player, &cpu);
    audio_unload();
    CloseWindow();
    return 0;
}