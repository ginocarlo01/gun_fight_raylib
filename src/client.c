#include "raylib.h"
#include "types.h"
#include "game_settings.h"
#include "input.h"
#include "render.h"
#include "audio.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main() {
    // Raylib Settings
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_UNDECORATED);
    InitWindow(ScreenDimensions.x, ScreenDimensions.y, "Gun Fight Client");
    SetTargetFPS(TargetFPS);
    init_audio();

    // Network Settings using 
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    GameState game_state = {0};
    InputPacket input_packet = {0};

    while (!WindowShouldClose()) {

        // INPUT 
        input_packet = read_input();

        sendto(sock, &input_packet, sizeof(input_packet), 0,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
        recvfrom(sock, &game_state, sizeof(game_state),MSG_DONTWAIT, NULL, NULL);

        // DRAW 
        BeginDrawing();

        ClearBackground(BLACK);
        draw_entities(&game_state);
        DrawText(TextFormat("P1: %d | P2: %d",game_state.player_score,game_state.cpu_score),10, 10, 20, WHITE);

        EndDrawing();
    }

    close_audio();
    CloseWindow();
    close(sock);
    return 0;
}
