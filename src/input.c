#include "input.h"

InputPacket read_input() {
    InputPacket input = {0};

    // ----- KEYBOARD -----
    input.up    = (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP));
    input.down  = (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
    input.left  = (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    input.right = (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
    input.shoot = IsKeyPressed(KEY_SPACE);

    // ----- GAMEPAD -----
    if (IsGamepadAvailable(0)) {
        input.shoot = IsGamepadButtonPressed(0, 7);

        float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float axisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        input.up    = axisY < -0.1f;
        input.down  = axisY >  0.1f;
        input.left  = axisX < -0.1f;
        input.right = axisX >  0.1f;
    }

    return input;
}
