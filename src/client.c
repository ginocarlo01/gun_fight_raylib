#include "raylib.h"
#include "types.h"
#include "game_settings.h"
#include "input.h"
#include "render.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main() {
    InitWindow(ScreenDimensions.x, ScreenDimensions.y, "Gun Fight Online");
    SetTargetFPS(TargetFPS);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    GameState state = {0};
    InputPacket input = {0};

    while (!WindowShouldClose()) {

        // --- INPUT ---
        input = read_input();   // agora lê teclado + gamepad corretamente

        sendto(sock, &input, sizeof(input), 0,(struct sockaddr *)&serverAddr, sizeof(serverAddr));

        ssize_t bytes = recvfrom(sock, &state, sizeof(state),MSG_DONTWAIT, NULL, NULL);

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);
        draw_entities(&state);

        DrawText(TextFormat("P1: %d | P2: %d",state.player_score,state.cpu_score),10, 10, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    close(sock);
    return 0;
}
