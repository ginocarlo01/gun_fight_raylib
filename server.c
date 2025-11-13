#include "raylib.h"
#include "types.h"
#include "config.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

#define MAX_PLAYERS 2

int main() {
    // Inicializa Raylib (janela invisível)
    SetConfigFlags(FLAG_WINDOW_HIDDEN);  // mantém janela oculta
    InitWindow(800, 450, "hidden window");
    SetTargetFPS(60);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr, clients[MAX_PLAYERS];
    socklen_t addrLen = sizeof(struct sockaddr_in);

    memset(clients, 0, sizeof(clients));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {perror("bind error");return 1;}

    GameState state = {0};
    InputPacket inputs[MAX_PLAYERS] = {0};

    float tick = 1.0f / 60.0f;
    double last = GetTime();
    bool game_started = false;

    //TODO if one player disconnects, stops the whole game
    while (!WindowShouldClose()) {
        InputPacket packet;
        struct sockaddr_in sender;
        ssize_t bytes = recvfrom(sock, &packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr*)&sender, &addrLen);

        if (bytes <= 0)
            continue;

        int player_id = -1;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (memcmp(&clients[i], &sender, sizeof(sender)) == 0) {
                player_id = i;
                break;
            }
        }

        // Novo cliente?
        if (player_id == -1) {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (clients[i].sin_port == 0) {
                    clients[i] = sender;
                    printf("New Player: %d (port %d)\n", i + 1, ntohs(sender.sin_port));
                    player_id = i;
                    break;
                }
            }

            //TODO check this conditional
            if (clients[0].sin_port != 0 && clients[1].sin_port != 0 && !game_started) {
                printf("Start Game\n");
                restart_game(state.entities);
                state.player_score = 0;
                state.cpu_score = 0;
                game_started = true;
            }
        }

        if (player_id != -1) inputs[player_id] = packet;

        if (!game_started) continue;

        //TODO check tick
        double now = GetTime();
        if (now - last >= tick) {
            last = now;

            // Aplica inputs
            for (int i = 0; i < MAX_PLAYERS; i++) {
                Entity *p = &state.entities[i];
                p->direction = (Vector2){0, 0};
                if (inputs[i].up) p->direction.y = -1;
                if (inputs[i].down) p->direction.y = 1;
                if (inputs[i].left) p->direction.x = -1;
                if (inputs[i].right) p->direction.x = 1;
                normalize(&p->direction);
                if (inputs[i].shoot) spawn_bullet(p, state.entities, MAX_ENTITIES);
                    
            }

            for (int i = 0; i < MAX_ENTITIES; i++)
                update_entity(&state.entities[i], tick);

            handle_bullet_collisions(state.entities, MAX_ENTITIES, &state.player_score, &state.cpu_score);

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (clients[i].sin_port != 0) {
                    ssize_t sent = sendto(sock, &state, sizeof(state), 0, (struct sockaddr*)&clients[i], addrLen);
                    if (sent < 0) perror("Error");
                }
            }
        }
    }

    CloseWindow();
    close(sock);
    return 0;
}
