#ifndef GAME_H
#define GAME_H

#include "renderer.h"
#include "texture.h"
#include "vec.h"
#include "map.h"
#include <SDL2/SDL_pixels.h>
#include <stdbool.h>

void game_init();

void game_run();
void game_pause();
void game_resume();
void game_stop();

void game_render();
void game_update();

void game_quit();

enum State {
	STOPPED,
	PAUSED,
	RUNNING
};

extern enum State current_state;

extern struct RawTexture *font_texture;
extern struct RawTexture *textures[TEXTURE_COUNT];

struct GameCamera {
	struct v2f pos;
	struct v2f dir;
	struct v2f plane;
};

extern struct GameCamera camera;

extern float current_time;
extern float last_time;
extern float delta_time;

extern float move_speed;
extern float rotate_speed;

extern bool KEYS[322];
extern char MAP[MAP_HEIGHT][MAP_WIDTH];

#endif /** GAME_H */
