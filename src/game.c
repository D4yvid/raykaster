#include "game.h"
#include "macros.h"
#include "raw_texture.h"
#include "texture.h"
#include "font.h"
#include "textures.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void game_handle_events();
static void game_handle_keyboard();
static void game_handle_mouse(SDL_MouseMotionEvent *event);
static void game_handle_collision(struct v2f new_pos);

enum State current_state;

struct RawTexture *textures[TEXTURE_COUNT];
struct RawTexture *font_texture;

struct GameCamera camera;

float current_time;
float last_time;
float delta_time;

float move_speed;
float rotate_speed;

bool KEYS[322];
bool mouse_fixed;
bool sprinting;

void game_init()
{
	SDL_Surface *texture_atlas_surface;
	SDL_Surface *font_texture_surface;
	int i;

	current_state = STOPPED;

	font_texture_surface = SDL_CreateRGBSurfaceWithFormatFrom(
		font_data.pixels,
		font_data.width,
		font_data.height,
		32,
		font_data.width * sizeof(unsigned),
		SDL_PIXELFORMAT_RGBA8888
	);

	texture_atlas_surface = SDL_CreateRGBSurfaceWithFormatFrom(
		textures_data.pixels,
		textures_data.width,
		textures_data.height,
		32,
		textures_data.width * sizeof(unsigned),
		SDL_PIXELFORMAT_RGBA8888
	);

	for (i = 0; i < TEXTURE_COUNT; i++)
	{
		textures[i] = raw_texture_from_surface_slice(
			texture_atlas_surface, 
			TEXTURE_WIDTH * i,
			TEXTURE_HEIGHT * 0,
			TEXTURE_WIDTH, 
			TEXTURE_HEIGHT
		);
	}

	font_texture = raw_texture_from_surface(font_texture_surface);

	mouse_fixed = true;

	SDL_FreeSurface(texture_atlas_surface);
	SDL_FreeSurface(font_texture_surface);

	SDL_CaptureMouse(mouse_fixed);

	memset(KEYS, 0, sizeof(KEYS));

	camera.dir.x = 0.63;
	camera.dir.y = -0.77;
	camera.pos.x = 22.13;
	camera.pos.y = 6.91;
	camera.plane.x = -0.59;
	camera.plane.y = -0.49;
}

void game_run()
{
	if (current_state & RUNNING)
	{
		ERROR("the game is already running");
		return;
	}

	current_state = RUNNING;

	current_time = SDL_GetPerformanceCounter();
	last_time = 0;

	while (current_state != STOPPED)
	{
		last_time = current_time;
		current_time = SDL_GetPerformanceCounter();

		delta_time = (float) ((current_time - last_time) * 1000 / (float) SDL_GetPerformanceFrequency());
		move_speed = (delta_time / 1000.f) / (sprinting ? .1 : .4);
		rotate_speed = (delta_time / 1000.f) / 8;

		game_handle_events();
		game_handle_keyboard();

		renderer_clear_screen();

		game_render();

		renderer_draw_screen();
	}
}

void game_pause()
{
	current_state |= PAUSED;
}

void game_resume()
{
	current_state ^= PAUSED;
}

void game_stop()
{
	current_state = STOPPED;
}

void game_quit()
{
	int i;

	raw_texture_destroy(font_texture);

	for (i = 0; i < sizeof(textures) / sizeof(textures[0]); i++)
	{
		raw_texture_destroy(textures[i]);
	}
}

static void game_handle_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			game_stop();
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym > 322) break;
			KEYS[event.key.keysym.sym] = true;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym > 322) break;
			KEYS[event.key.keysym.sym] = false;
			break;
		case SDL_MOUSEMOTION:
			game_handle_mouse(&event.motion);
			break;
		default:
			break;
		}
	}
}

static void game_handle_keyboard()
{
	struct v2f temp_pos = camera.pos;
	struct v2f temp_dir = camera.dir;

	/** Movement keys */
	if (KEYS[SDLK_w])
	{
		temp_pos.x += camera.dir.x * move_speed;
		temp_pos.y += camera.dir.y * move_speed;
		game_handle_collision(temp_pos);
	}

	if (KEYS[SDLK_s])
	{
		temp_pos.x -= camera.dir.x * move_speed;
		temp_pos.y -= camera.dir.y * move_speed;
		game_handle_collision(temp_pos);
	}

	if (KEYS[SDLK_a] && !KEYS[SDLK_d])
	{
		temp_dir.x = -camera.dir.y;
		temp_dir.y = camera.dir.x;

		temp_pos.x += temp_dir.x * move_speed;
		temp_pos.y += temp_dir.y * move_speed;

		game_handle_collision(temp_pos);
	}

	if (KEYS[SDLK_d] && !KEYS[SDLK_a])
	{
		temp_dir.x = camera.dir.y;
		temp_dir.y = -camera.dir.x;

		temp_pos.x += temp_dir.x * move_speed * 0.65;
		temp_pos.y += temp_dir.y * move_speed * 0.65;

		game_handle_collision(temp_pos);
	}

	sprinting = KEYS[SDLK_r];

	if (KEYS[SDLK_ESCAPE])
	{
		game_stop();
	}
}

static void game_handle_collision(struct v2f new_pos)
{
	if (
		!(MAP[(int) new_pos.x][(int) new_pos.y] >0) &&
		!(new_pos.x > MAP_HEIGHT || new_pos.x <= 1) &&
		!(new_pos.y > MAP_WIDTH || new_pos.y <= 1)
	) {
		camera.pos.x = new_pos.x;
		camera.pos.y = new_pos.y;
	}
}

static void game_handle_mouse(SDL_MouseMotionEvent *event)
{
	if (event->xrel < 0)
	{
		float old_dir_x = camera.dir.x;
		float old_plane_x = camera.plane.x;

		camera.dir.x = camera.dir.x * cos(rotate_speed * -event->xrel) - camera.dir.y * sin(rotate_speed * -event->xrel);
		camera.dir.y = old_dir_x * sin(rotate_speed * -event->xrel) + camera.dir.y * cos(rotate_speed * -event->xrel);

		camera.plane.x = camera.plane.x * cos(rotate_speed * -event->xrel) - camera.plane.y * sin(rotate_speed * -event->xrel);
		camera.plane.y = old_plane_x * sin(rotate_speed * -event->xrel) + camera.plane.y * cos(rotate_speed * -event->xrel);
	}
	else {
		float old_dir_x = camera.dir.x;
		float old_plane_x = camera.plane.x;

		camera.dir.x = camera.dir.x * cos(-(rotate_speed * event->xrel)) - camera.dir.y * sin(-(rotate_speed * event->xrel));
		camera.dir.y = old_dir_x * sin(-(rotate_speed * event->xrel)) + camera.dir.y * cos(-(rotate_speed * event->xrel));

		camera.plane.x = camera.plane.x * cos(-(rotate_speed * event->xrel)) - camera.plane.y * sin(-(rotate_speed * event->xrel));
		camera.plane.y = old_plane_x * sin(-(rotate_speed * event->xrel)) + camera.plane.y * cos(-(rotate_speed * event->xrel));
	}
}
