#include "render.h"

void draw_entities(const GameState *state) {
    for (int i = 0; i < state->entities_qty; i++) {
        Entity e = state->entities[i];
        if (!e.enabled) continue;
        DrawCircleV(e.position, e.radius, e.color);
    }
}
