#include "entities.h"
#include <raylib.h>

const Entity DefaultPlayer = {
    .type = PLAYER,
    .position = { 100, 270 },
    .color = BLUE,
    .speed = 200,
    .radius = 20,
    .ammo = 5,
    .enabled = true
};

const Entity DefaultCPU = {
    .type = CPU,
    .position = { 860, 270 },
    .direction = { 1, 1 },
    .color = RED,
    .speed = 150,
    .radius = 20,
    .ammo = 5,
    .recharge_time = 1,
    .enabled = true
};

const Entity DefaultBulletOfPlayer = {
    .type = BULLET,
    .color = BLUE,
    .speed = 300,
    .radius = 10,
    .owner = PLAYER
};

const Entity DefaultBulletOfCPU = {
    .type = BULLET,
    .color = RED,
    .speed = 300,
    .radius = 10,
    .owner = CPU
};

const Entity ObstacleDestroyBullet = {
    .type = OBSTACLE,
    .behaviour = DESTROY_BULLET_ONLY,
    .direction = { 0, 1 },
    .color = WHITE,
    .speed = 150,
    .radius = 15,
    .enabled = true
};

const Entity ObstacleDamageOwner = {
    .type = OBSTACLE,
    .behaviour = DAMAGE_OWNER,
    .direction = { 0, 1 },
    .color = RED,
    .speed = 50,
    .radius = 15,
    .enabled = true
};

const Entity ObstacleGiveAmmoOwner = {
    .type = OBSTACLE,
    .behaviour = GIVE_AMMO_OWNER,
    .direction = { 0, 1 },
    .color = GREEN,
    .speed = 200,
    .radius = 15,
    .enabled = true
};

const Entity ObstaclesOrder[] = {
    ObstacleGiveAmmoOwner,
    ObstacleDamageOwner,
    ObstacleGiveAmmoOwner,
    ObstacleDestroyBullet
};

//TODO check exaclty why this works only here
const int ObstaclesOrderSize = sizeof(ObstaclesOrder) / sizeof(ObstaclesOrder[0]);
