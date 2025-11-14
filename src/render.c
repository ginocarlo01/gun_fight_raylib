#include "render.h"
#include "types.h"
#include "game_settings.h"
#include <stdio.h>


const char*  PlayerTexturePath = "assets/player.png";
const char*  CPUTexturePath = "assets/enemy.png";
const char*  BulletTexturePath = "assets/bullet.png"; 
const char*  DefaultObstacleTexturePath = "assets/default_object.png";
const char*  BadObstacleTexturePath = "assets/bad_object.png";
const char*  GoodObstacleTexturePath = "assets/life_object.png";
const char*  HeartTexturePath = "assets/heart.png";;

Texture2D PlayerTexture;
Texture2D CPUTexture;
Texture2D BulletTexture;
Texture2D DefaultObstacleTexture;
Texture2D BadObstacleTexture;
Texture2D GoodObstacleTexture;

void init_textures() {
    PlayerTexture = LoadTexture(PlayerTexturePath);
    CPUTexture = LoadTexture(CPUTexturePath);
    BulletTexture = LoadTexture(BulletTexturePath);
    DefaultObstacleTexture = LoadTexture(DefaultObstacleTexturePath);
    BadObstacleTexture = LoadTexture(BadObstacleTexturePath);
    GoodObstacleTexture = LoadTexture(GoodObstacleTexturePath);

    // Debug: print sizes to confirm load
    // printf("[render] PlayerTexture: %d x %d\n", PlayerTexture.width, PlayerTexture.height);
    // printf("[render] CPUTexture: %d x %d\n", CPUTexture.width, CPUTexture.height);
    // printf("[render] BulletTexture: %d x %d\n", BulletTexture.width, BulletTexture.height);
    // printf("[render] DefaultObstacleTexture: %d x %d\n", DefaultObstacleTexture.width, DefaultObstacleTexture.height);
    // printf("[render] BadObstacleTexture: %d x %d\n", BadObstacleTexture.width, BadObstacleTexture.height);
    // printf("[render] GoodObstacleTexture: %d x %d\n", GoodObstacleTexture.width, GoodObstacleTexture.height);
}

void unload_textures() {
    UnloadTexture(PlayerTexture);
    UnloadTexture(CPUTexture);
    UnloadTexture(BulletTexture);
    UnloadTexture(DefaultObstacleTexture);
    UnloadTexture(BadObstacleTexture);
    UnloadTexture(GoodObstacleTexture);
}

void draw_entities(const GameState *game_state) {
    for (int i = 0; i < game_state->entities_qty; i++) {
        Entity e = game_state->entities[i];
        if (!e.enabled) continue;
        Texture2D tex;
        switch (e.type) {
            case PLAYER: tex = PlayerTexture; break;
            case CPU: tex = CPUTexture; break;
            case BULLET: tex = BulletTexture; break;
            case OBSTACLE:
                switch (e.behaviour) {
                    case DESTROY_BULLET_ONLY: tex = DefaultObstacleTexture; break;

                    case DAMAGE_OWNER: tex = BadObstacleTexture; break;

                    case GIVE_AMMO_OWNER: tex = GoodObstacleTexture; break;
                }
                break;
        }

        DrawTextureEx(tex, (Vector2){ e.position.x - tex.width, e.position.y - tex.height}, 0, 2, WHITE);

        //For Debug:
        //DrawCircleV(e.position, e.radius, e.color);
    }

    for (int i = 0; i < game_state->entities[0].ammo; i++) 
        DrawTexture(BulletTexture, i * BulletTexture.width, ScreenDimensions.y - BulletTexture.height, WHITE);
    
    for (int i = 0; i < game_state->entities[1].ammo; i++) 
        DrawTexture(BulletTexture, ScreenDimensions.x - BulletTexture.width - i * BulletTexture.width, ScreenDimensions.y - BulletTexture.height, WHITE);
}
