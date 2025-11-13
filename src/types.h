#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <raylib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef u8 percentage;
typedef u8 seconds;
typedef struct Vec2 {
    u16 x;
    u16 y;} Vec2;

typedef enum {
    CPU,
    PLAYER,
    BULLET,
    OBSTACLE
} EntityType;

typedef enum {
    DAMAGE_OWNER,
    DESTROY_BULLET_ONLY,
    GIVE_AMMO_OWNER
} EntityBehaviour;

typedef struct Entity {
    EntityType type;
    Vector2 position;
    Vector2 direction;
    u8 radius;
    u8 ammo;
    seconds recharge_time;
    EntityBehaviour behaviour;
    EntityType owner;
    u16 speed;
    Color color;
    bool enabled;
} Entity;

#endif