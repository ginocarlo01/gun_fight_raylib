
#include "types.h"
#include "config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "raylib.h"

const size_t OBSTACLES_QTY = 4;

void normalize(Vector2 *v) {
    float mag = sqrtf(v->x*v->x + v->y*v->y);
    if (mag > 0) { v->x /= mag; v->y /= mag; }
}

void update_entity(Entity *entity, float delta) {
    if (!entity->enabled) return;
    entity->position.x += delta * entity->speed * entity->direction.x;
    entity->position.y += delta * entity->speed * entity->direction.y;
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

// void spawn_bullet(Entity *entity, Entity *entities, int entities_qty) {
//     printf("[spawn_bullet] Chamado por entidade tipo=%d (ammo=%d)\n", entity->type, entity->ammo);

//     if (entity->ammo <= 0) {
//         printf("[spawn_bullet] Sem munição!\n");
//         return;
//     }

//     for (int i = entities_qty - 1; i >= 0; i--) {
//         printf("[spawn_bullet] Checando entidade %d (owner=%d, enabled=%d, pos=(%.1f, %.1f))\n",
//                i, entities[i].owner, entities[i].enabled, entities[i].position.x, entities[i].position.y);

//         if (entities[i].owner != entity->type) {
//             printf("  -> Owner diferente (precisa ser %d)\n", entity->type);
//             continue;
//         }
//         if (entities[i].position.x != 0 && entities[i].position.y != 0) {
//             printf("  -> Já tem posição usada (%.1f, %.1f)\n", entities[i].position.x, entities[i].position.y);
//             continue;
//         }
//         if (entities[i].enabled) {
//             printf("  -> Já está habilitada, pulando...\n");
//             continue;
//         }

//         printf("[spawn_bullet] Encontrada entidade livre no slot %d, criando bala!\n", i);

//         entities[i].enabled = true;
//         entities[i].position = (Vector2){
//             entity->position.x + (entity->radius + 10) * (entity->type == PLAYER ? 1 : -1),
//             entity->position.y
//         };
//         entities[i].direction.x = (entity->type == PLAYER ? 1 : -1);
//         entity->ammo--;

//         printf("[spawn_bullet] Bala criada! pos=(%.1f, %.1f), dir.x=%.1f, ammo restante=%d\n",
//                entities[i].position.x, entities[i].position.y, entities[i].direction.x, entity->ammo);
//         return;
//     }

//     printf("[spawn_bullet] Nenhuma entidade disponível para bala!\n");
// }


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

void handle_bullet_collisions(Entity *entities, int entities_qty, u8 *player_score, u8 *cpu_score) {
    for (int bullet_idx = 2 + OBSTACLES_QTY; bullet_idx < entities_qty; bullet_idx++) {
        if (!entities[bullet_idx].enabled) continue;

        for (int target_idx = 0; target_idx < 2 + OBSTACLES_QTY; target_idx++) {
            if (!entities[target_idx].enabled) continue;
            if (!CheckCollisionCircles(entities[bullet_idx].position, entities[bullet_idx].radius,entities[target_idx].position, entities[target_idx].radius)) continue;
            if (entities[bullet_idx].owner == PLAYER && entities[target_idx].type == CPU) {
                (*player_score)++;
                PlaySound(player_win_sfx);
                restart_game(entities);
            } else if (entities[bullet_idx].owner == CPU && entities[target_idx].type == PLAYER) {
                (*cpu_score)++;
                PlaySound(player_lose_sfx);
                restart_game(entities);
            } else if (entities[target_idx].behaviour == DESTROY_BULLET_ONLY) {
                PlaySound(ball_hit_sfx);
                entities[bullet_idx].enabled = entities[target_idx].enabled = false;
            } else if (entities[target_idx].behaviour == DAMAGE_OWNER) {
                (entities[bullet_idx].owner == PLAYER ? (*cpu_score)++ : (*player_score)++);
                restart_game(entities);
            }
        }
    }
}
