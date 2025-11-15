#include "input.h"

InputPacket read_input() {
    InputPacket input_packet = {0};

    // KEYBOARD
    input_packet.up    = (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP));
    input_packet.down  = (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
    input_packet.left  = (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    input_packet.right = (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
    input_packet.shoot = IsKeyPressed(KEY_SPACE);

    // GAMEPAD
    if (IsGamepadAvailable(0)) {
        input_packet.shoot = IsGamepadButtonPressed(0, 7);

        float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float axisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        input_packet.up    = axisY < -0.1f;
        input_packet.down  = axisY >  0.1f;
        input_packet.left  = axisX < -0.1f;
        input_packet.right = axisX >  0.1f;
    }

    return input_packet;
}
