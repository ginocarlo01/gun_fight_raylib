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
#include <time.h>

int main() {
    // Raylib Settings
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(800, 450, "");
    SetTargetFPS(TargetFPS);
    srand((unsigned int) clock());

    // Network Settings
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr, clients[MAX_PLAYERS_ONLINE_MODE];
    socklen_t addrLen = sizeof(struct sockaddr_in);
    memset(clients, 0, sizeof(clients));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind error");
        return 1;
    }

    // Game Settings
    GameState game_state = {0};
    InputPacket inputs[MAX_PLAYERS_ONLINE_MODE] = {0};
    double tick = 1.0 / TargetFPS;
    double last = GetTime();
    bool game_started = false;

    while (!WindowShouldClose()) {

        InputPacket packet;
        struct sockaddr_in sender;
        ssize_t bytes = recvfrom(sock, &packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr*)&sender, &addrLen);

        if (bytes > 0) {
            int player_id = -1;

            // defines which player
            for (int i = 0; i < MAX_PLAYERS_ONLINE_MODE; i++) {
                if (memcmp(&clients[i], &sender, sizeof(sender)) == 0) {
                    player_id = i;
                    break;
                }
            }

            // if the player was not defined
            if (player_id == -1) {
                for (int i = 0; i < MAX_PLAYERS_ONLINE_MODE; i++) {
                    if (clients[i].sin_port == 0) {
                        clients[i] = sender;
                        printf("New Player: %d\n", i + 1);
                        player_id = i;
                        break;
                    }
                }

                // when both player connect, start the game
                if (clients[0].sin_port != 0 && clients[1].sin_port != 0 && !game_started) {
                    printf("Starting game\n");
                    restart_game(&game_state);
                    game_state.player_score = 0;
                    game_state.cpu_score = 0;
                    game_started = true;
                }
            }
            else{
                inputs[player_id] = packet;
            }
        }

        if (!game_started) continue;

        //TODO check if it is possible to not use GetTime
        double now = GetTime();
        if (now - last < tick) continue;

        last = now;

        for (int i = 0; i < MAX_PLAYERS_ONLINE_MODE; i++) {
            game_state.entities[i].direction = process_input(i == 0? PLAYER : CPU, &game_state, inputs[i]);
        }

        check_end_of_match(&game_state);
        for (int i = 0; i < game_state.entities_qty; i++) update_entity(&game_state.entities[i], tick);
        handle_bullet_collisions(&game_state);

        for (int i = 0; i < MAX_PLAYERS_ONLINE_MODE; i++) {
            if (clients[i].sin_port != 0) sendto(sock, &game_state, sizeof(game_state), 0,(struct sockaddr*)&clients[i], addrLen);
        }
    }

    CloseWindow();
    close(sock);
    return 0;
}
