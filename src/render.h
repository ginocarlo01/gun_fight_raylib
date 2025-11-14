#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include <raylib.h>

extern const char*  PlayerTexturePath;
extern const char*  CPUTexturePath;
extern const char*  BulletTexturePath;
extern const char*  DefaultObstacleTexturePath;
extern const char*  BadObstacleTexturePath;
extern const char*  GoodObstacleTexturePath;

extern Texture2D PlayerTexture;
extern Texture2D CPUTexture;
extern Texture2D BulletTexture;
extern Texture2D DefaultObstacleTexture;
extern Texture2D BadObstacleTexture;
extern Texture2D GoodObstacleTexture;

void init_textures();
void unload_textures();
void draw_entities(const GameState *state);

#endif
