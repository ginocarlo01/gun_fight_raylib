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
#include <time.h>

int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_UNDECORATED);
    InitWindow(ScreenDimensions.x, ScreenDimensions.y, "");
    SetTargetFPS(TargetFPS);
    init_audio();
    init_textures();
    srand((unsigned int) clock());

    GameState game_state = {0};
    restart_game(&game_state);
    game_state.player_score = 0;
    game_state.cpu_score = 0;

    Entity *player = &game_state.entities[0];
    Entity *cpu = &game_state.entities[1];

    while (!WindowShouldClose()) {
        // INPUT
        InputPacket input = read_input();
        player->direction = process_input(player->type, &game_state, input);

        // UPDATE
        check_end_of_match(&game_state);
        auto_spawn_bullet(cpu, &game_state);
        for (int i = 0; i < game_state.entities_qty; i++) update_entity(&game_state.entities[i], GetFrameTime());
        handle_bullet_collisions(&game_state);

        // DRAW
        BeginDrawing();
        ClearBackground(BackgroundColor);
        draw_entities(&game_state);

        // DEBUG
        //DrawText(TextFormat("FPS: %i", GetFPS()), 10, 30, 20, GREEN);
        DrawText(TextFormat("Score: %i", game_state.player_score), 10, ScreenDimensions.y - 70, 20, BLUE);
        DrawText(TextFormat("Score: %i", game_state.cpu_score), ScreenDimensions.x - 100, ScreenDimensions.y - 70, 20, RED);

        EndDrawing();
    }
    unload_textures();
    close_audio();
    CloseWindow();
    return 0;
}
