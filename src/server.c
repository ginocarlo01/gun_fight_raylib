#include "raylib.h"
#include "types.h"
#include "game_settings.h"
#include "input.h"
#include "game_logic.h"
#include "entities.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAX_PLAYERS 2

int main() {
    // Raylib apenas para timing e math
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(800, 450, "Server Window Hidden");
    SetTargetFPS(TargetFPS);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr, clients[MAX_PLAYERS];
    socklen_t addrLen = sizeof(struct sockaddr_in);

    memset(clients, 0, sizeof(clients));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind error");
        return 1;
    }

    GameState state = {0};
    InputPacket inputs[MAX_PLAYERS] = {0};

    double tick = 1.0 / TargetFPS;
    double last = GetTime();
    bool game_started = false;

    while (!WindowShouldClose()) {

        // --- RECEIVE PACKET ---
        InputPacket packet;
        struct sockaddr_in sender;
        ssize_t bytes = recvfrom(sock, &packet, sizeof(packet),
                                 MSG_DONTWAIT,
                                 (struct sockaddr*)&sender,
                                 &addrLen);

        if (bytes > 0) {
            int pid = -1;

            // Verifica se é um jogador já registrado
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (memcmp(&clients[i], &sender, sizeof(sender)) == 0) {
                    pid = i;
                    break;
                }
            }

            // Novo jogador
            if (pid == -1) {
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (clients[i].sin_port == 0) {
                        clients[i] = sender;
                        printf("[SERVER] Novo jogador: %d\n", i + 1);
                        pid = i;
                        break;
                    }
                }

                if (clients[0].sin_port != 0 &&
                    clients[1].sin_port != 0 &&
                    !game_started) {
                    printf("[SERVER] Iniciando partida\n");
                    restart_game(&state);
                    state.player_score = 0;
                    state.cpu_score = 0;
                    game_started = true;
                }
            }

            if (pid != -1)
                inputs[pid] = packet;
        }

        if (!game_started)
            continue;

        // --- FIXED TICK ---
        double now = GetTime();
        if (now - last < tick)
            continue;

        last = now;

        // --- PROCESS INPUT ---
        for (int i = 0; i < 2; i++) {
            Entity *p = &state.entities[i];
            p->direction = process_input(&state, inputs[i]);
        }

        // --- UPDATE ENTITIES ---
        for (int i = 0; i < state.entities_qty; i++)
            update_entity(&state.entities[i]);

        handle_bullet_collisions(&state);

        // --- SEND STATE ---
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clients[i].sin_port != 0) {
                sendto(sock, &state, sizeof(state), 0,
                       (struct sockaddr*)&clients[i], addrLen);
            }
        }
    }

    CloseWindow();
    close(sock);
    return 0;
}
