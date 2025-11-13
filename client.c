#include "raylib.h"
#include "types.h"
#include "config.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "common.h"


int main() {
    InitWindow(SCREEN_DIMENSIONS.x, SCREEN_DIMENSIONS.y, "Pong Online");
    SetTargetFPS(TARGET_FPS);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    GameState state = {0};

    while (!WindowShouldClose()) {
        InputPacket input = {0};
        input.up = IsKeyDown(KEY_UP);
        input.down = IsKeyDown(KEY_DOWN);
        input.left = IsKeyDown(KEY_LEFT);
        input.right = IsKeyDown(KEY_RIGHT);
        input.shoot = IsKeyPressed(KEY_SPACE);

        sendto(sock, &input, sizeof(input), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        ssize_t bytes = recvfrom(sock, &state, sizeof(state), MSG_DONTWAIT, NULL, NULL);
        if (bytes > 0) printf("[CLIENT] Recebidos %zd bytes do servidor\n", bytes);
        
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (state.entities[i].enabled)
                DrawCircle(state.entities[i].position.x, state.entities[i].position.y, state.entities[i].radius, state.entities[i].color);
        }

        DrawText(TextFormat("P1: %d | P2: %d", state.player_score, state.cpu_score), 10, 10, 20, WHITE);
        EndDrawing();
    }

    CloseWindow();
    close(sock);
    return 0;
}
