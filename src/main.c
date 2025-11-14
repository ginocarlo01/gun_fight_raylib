#include "raylib.h"
#include "types.h"
#include "audio.h"
#include "game_settings.h"
#include "game_logic.h"
#include "render.h"
#include "entities.h"
#include "input.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <raymath.h>

int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_UNDECORATED);
    InitWindow(ScreenDimensions.x, ScreenDimensions.y, "");
    SetTargetFPS(TargetFPS);
    init_audio();
    init_textures();

    GameState game = {0};
    restart_game(&game);
    game.player_score = 0;
    game.cpu_score = 0;

    Entity *player = &game.entities[0];
    Entity *cpu = &game.entities[1];

    while (!WindowShouldClose()) {
        // INPUT
        InputPacket input = read_input();
        player->direction = process_input(player->type, &game, input);

        // UPDATE
        auto_spawn_bullet(cpu, game.entities, game.entities_qty);
        for (int i = 0; i < game.entities_qty; i++) update_entity(&game.entities[i], GetFrameTime());
        handle_bullet_collisions(&game);

        // DRAW
        BeginDrawing();
        ClearBackground(BackgroundColor);
        draw_entities(&game);

        // DEBUG
        DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);
        DrawText(TextFormat("Player Score: %i", game.player_score), 10, 50, 20, WHITE);
        DrawText(TextFormat("CPU Score: %i", game.cpu_score), 10, 70, 20, WHITE);
        DrawText(TextFormat("Player Ammo: %i", player->ammo), 10, 90, 20, WHITE);

        EndDrawing();
    }
    unload_textures();
    close_audio();
    CloseWindow();
    return 0;
}
