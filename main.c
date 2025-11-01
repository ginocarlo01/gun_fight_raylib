#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*==============================================================================
    CONFIG
==============================================================================*/

static const Vector2 AIM_DIRECTIONS[3] = {
    {1, 0},
    {1, -1},
    {1, 1}  
};

static const int INITIAL_AIM_INDEX = 0;
static const int LENGTH_LINE_AIM = 40;
static const size_t AIM_DIRECTIONS_LEN = sizeof(AIM_DIRECTIONS) / sizeof(AIM_DIRECTIONS[0]);
static const float LIMIT_PLAYER_LEFT = 0.05f;
static const float LIMIT_PLAYER_RIGHT = 0.30f;
static const float LIMIT_OBSTACLE_LEFT = 0.30f;
static const float LIMIT_OBSTACLE_RIGHT = 0.70f;
static const float LIMIT_CPU_LEFT = 0.7f;
static const float LIMIT_CPU_RIGHT = 0.95f;
static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;
static const char * GAME_TITLE = "Gun Fight";
static const int TARGET_FPS = 240;

static const float PLAYER_START_POSITION_X = LIMIT_PLAYER_LEFT * 5 * SCREEN_WIDTH;
static const float PLAYER_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color PLAYER_COLOR = BLUE;
static const int PLAYER_RADIUS = 20;
static const int PLAYER_SPEED = 300;
static const int PLAYER_DAMAGE = 0;
static const int PLAYER_AMMO = 5;
static const int PLAYER_LIFE = 1;
//Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){70, SCREEN_HEIGHT/2}, BLUE, 20, 300, 0, 5, 1,true);

static const Color PLAYER_BULLET_COLOR = GREEN;
static const int PLAYER_BULLET_RADIUS = 10;
static const int PLAYER_BULLET_SPEED = 400;
static const int PLAYER_BULLET_DAMAGE = 1;

static const float CPU_START_POSITION_X = SCREEN_WIDTH * 0.85f;
static const float CPU_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color CPU_COLOR = RED;
static const int CPU_RADIUS = 20;
static const int CPU_SPEED = 200;
static const int CPU_DAMAGE = 0;
static const int CPU_AMMO = 15;
static const int CPU_LIFE = 1;

static const Color CPU_BULLET_COLOR = RED;
static const int CPU_BULLET_RADIUS = 10;
static const int CPU_BULLET_SPEED = 400;
static const int CPU_BULLET_DAMAGE = 1;

static const int QTY_OBSTACLES = 4;

//default obstacle
static const float OBSTACLE_START_POSITION_Y = SCREEN_HEIGHT * 0.5;
static const Color OBSTACLE_COLOR = WHITE;
static const int OBSTACLE_RADIUS = 20;
static const int OBSTACLE_SPEED = 200;
static const int OBSTACLE_DAMAGE = 0;

//damage obstacle
static const Color OBSTACLE_DAMAGE_COLOR = RED;
static const int OBSTACLE_DAMAGE_RADIUS = 20;
static const int OBSTACLE_DAMAGE_SPEED = 250;
static const int OBSTACLE_DAMAGE_DAMAGE = 1;

//healing obstacle
static const Color OBSTACLE_HEALING_COLOR = GREEN;
static const int OBSTACLE_HEALING_RADIUS = 20;
static const int OBSTACLE_HEALING_SPEED = 200;
static const int OBSTACLE_HEALING_DAMAGE = -1;

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

void normalize(Vector2 *v) {
    float mag = sqrtf(v->x * v->x + v->y * v->y);
    if (mag != 0.0f) {
        v->x /= mag;
        v->y /= mag;
    }
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
    return e;
}

void spawn_bullet(Entity *entity) {
    if (entity->ammo <= 0) return;

    Vector2 dir = AIM_DIRECTIONS[entity->aim_index];
    normalize(&dir);

    float line_len = LENGTH_LINE_AIM;

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
        if (entity->aim_index < 0) entity->aim_index = AIM_DIRECTIONS_LEN - 1;

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

        float limitLeft  = LIMIT_PLAYER_LEFT * SCREEN_WIDTH;
        float limitRight = LIMIT_PLAYER_RIGHT * SCREEN_WIDTH;

        if (entity->position.x < limitLeft)  entity->position.x = limitLeft;
        if (entity->position.x > limitRight) entity->position.x = limitRight;
    } else if (entity->type == ENTITY_CPU) {
        if (entity->position.y < 0){
            entity->position.y = 0;
            entity->direction.y *= -1;
        }

        if (entity->position.y > SCREEN_HEIGHT){
            entity->position.y = SCREEN_HEIGHT;
            entity->direction.y *= -1;
        }
        

        float limitLeft  = LIMIT_CPU_LEFT * SCREEN_WIDTH; 
        float limitRight = LIMIT_CPU_RIGHT * SCREEN_WIDTH;  
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


void draw_entity(Entity *e) {
    if (!e->enabled) return;

    DrawCircleV(e->position, e->radius, e->color);

    //TODO juntar em uma soh area
    if (e->type == ENTITY_PLAYER) {
        float line_len = LENGTH_LINE_AIM; 
        for (int i = 0; i < AIM_DIRECTIONS_LEN; i++) {
            Color col = (i == e->aim_index) ? YELLOW : GRAY;
            Vector2 end = { e->position.x + AIM_DIRECTIONS[i].x * line_len,
                            e->position.y + AIM_DIRECTIONS[i].y * line_len };
            DrawLineV(e->position, end, col);
            }
    }
    if (e->type == ENTITY_CPU) {
        float line_len = LENGTH_LINE_AIM; 
        for (int i = 0; i < AIM_DIRECTIONS_LEN; i++) {
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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    //PLAYER
    Entity player = create_entity(ENTITY_PLAYER, (Vector2){0,0}, (Vector2){PLAYER_START_POSITION_X, PLAYER_START_POSITION_Y}, PLAYER_COLOR, PLAYER_RADIUS, PLAYER_SPEED, PLAYER_DAMAGE, PLAYER_AMMO, PLAYER_LIFE,true);
    //TODO checar se eh possivel otimizar
    player.bullets = malloc(sizeof(Entity) * PLAYER_AMMO);
    for (int i = 0; i < PLAYER_AMMO; i++) {
        player.bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, PLAYER_BULLET_COLOR, PLAYER_BULLET_RADIUS, PLAYER_BULLET_SPEED, PLAYER_BULLET_DAMAGE, 0, 0, false);
    }

    //CPU
    //TODO verificar se eh possivel colocar em uma funcao
    Entity cpu_bullets[15]; 
    Entity cpu = create_entity(ENTITY_CPU, (Vector2){0,1}, (Vector2){CPU_START_POSITION_X, CPU_START_POSITION_Y}, CPU_COLOR, CPU_RADIUS, CPU_SPEED, CPU_DAMAGE, CPU_AMMO, CPU_LIFE, true);
    cpu.ammo = 15;
    //TODO checar se eh possivel otimizar
    cpu.bullets = cpu_bullets;
    for (int i = 0; i < cpu.ammo; i++) {
        cpu.bullets[i] = create_entity(ENTITY_BULLET, (Vector2){0,0}, (Vector2){0,0}, CPU_BULLET_COLOR, CPU_BULLET_RADIUS, CPU_BULLET_SPEED, CPU_BULLET_DAMAGE, 0, 0, false);
    
    }

    int num_obstacles = QTY_OBSTACLES;
    Entity obstacles[num_obstacles];
    float startX = LIMIT_OBSTACLE_LEFT * SCREEN_WIDTH;
    float endX   = LIMIT_OBSTACLE_RIGHT * SCREEN_WIDTH;
    float spacing = (endX - startX) / (num_obstacles + 1);

    for (int i = 0; i < num_obstacles; i++) {
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
        //DrawLine(70, 0, 2 + 70, SCREEN_HEIGHT, BLUE); //player start 
        int player_limit_left = 0.05f * SCREEN_WIDTH;
        int player_limit_right = 0.30f * SCREEN_WIDTH;
        int obstacle_limit_left = 0.31f * SCREEN_WIDTH;
        int obstacle_limit_right = 0.70f * SCREEN_WIDTH;
        int cpu_limit_left = 0.71f * SCREEN_WIDTH;
        int cpu_limit_right = 0.95f * SCREEN_WIDTH;
        DrawLine(player_limit_left, 0, player_limit_left, SCREEN_HEIGHT, BLUE); 
        DrawLine(player_limit_right, 0, player_limit_right, SCREEN_HEIGHT, BLUE); 
        DrawLine(obstacle_limit_left, 0, obstacle_limit_left, SCREEN_HEIGHT, WHITE); 
        DrawLine(obstacle_limit_right, 0, obstacle_limit_right, SCREEN_HEIGHT, WHITE); 
        DrawLine(cpu_limit_left, 0, cpu_limit_left, SCREEN_HEIGHT, RED); 
        DrawLine(cpu_limit_right, 0, cpu_limit_right, SCREEN_HEIGHT, RED); 
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
