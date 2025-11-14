#include "input.h"

InputPacket read_input(void) {
    InputPacket input = {0};

    // aqui você coloca exatamente o que já tinha
    // exemplo (substitua pelo seu):
    input.up = IsKeyDown(KEY_W);
    input.down = IsKeyDown(KEY_S);
    input.left = IsKeyDown(KEY_A);
    input.right = IsKeyDown(KEY_D);
    input.shoot = IsKeyDown(KEY_SPACE);

    return input;
}
