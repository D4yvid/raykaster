#include "renderer.h"
#include "game.h"
#include "macros.h"
#include "vec.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static bool initialized;

static SDL_Window *window;
static SDL_Surface *surface;

static struct {
	SDL_Color *colors;
	int start, count;
} pallete_info;

static struct v2_8 renderer_get_char_offset(char c);

void renderer_init()
{
	DEBUG("initializing SDL2");
	ASSERT(SDL_Init(SDL_INIT_EVERYTHING) == 0, "could not initialize SDL2: %s", SDL_GetError());

	DEBUG("initializing SDL2 image subsystem");
	ASSERT(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != 0, "could not initialize SDL2 image subsystem: %s", IMG_GetError());

	SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, 0);

	DEBUG("creating SDL window");
	ASSERT((window = SDL_CreateWindow(
			"game",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			FRAMEBUFFER_WIDTH * FRAMEBUFFER_PIXELSCALE,
			FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PIXELSCALE,
			SDL_WINDOW_OPENGL
		)) != NULL,
		"could not create SDL2 window: %s",
		SDL_GetError()
	);

	DEBUG("obtaining SDL window surface");
	ASSERT((surface = SDL_GetWindowSurface(window)) != NULL, "could not get window surface: %s", SDL_GetError());

	/** Enable relative mouse motion to .xrel and .yrel in SDL_MotionEvent */
	SDL_SetRelativeMouseMode(1);

	DEBUG("renderer initialized with success!");
	initialized = 1;
}

void renderer_set_pallete(SDL_Color *pallete, int start, int count)
{
	pallete_info.colors = pallete;
	pallete_info.count = count;
	pallete_info.start = start;

	SDL_SetPaletteColors(surface->format->palette, pallete, start, count);
}

void renderer_clear_screen()
{
	SDL_LockSurface(surface);

	SDL_memset(surface->pixels, 0, surface->h * surface->pitch);

	SDL_UnlockSurface(surface);
}

void renderer_draw_screen()
{
	SDL_UpdateWindowSurface(window);
}

void renderer_draw_raw_texture(struct RawTexture *texture, SDL_Rect *slice, SDL_Rect *pos)
{
	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
		texture->pixels,
		texture->format == FORMAT_XY ? texture->width : texture->height,
		texture->format == FORMAT_XY ? texture->height : texture->width,
		32,
		(texture->format == FORMAT_XY ? texture->width : texture->height) * sizeof(int),
		SDL_PIXELFORMAT_RGBA8888);

	renderer_draw_surface(surface, slice, pos);

	SDL_FreeSurface(surface);
}

void renderer_draw_surface(SDL_Surface *_surface, SDL_Rect *slice, SDL_Rect *pos)
{
	SDL_BlitScaled(_surface, slice, surface, pos);
}

void renderer_draw_line(int color, int x0, int y0, int x1, int y1)
{
	/** testing */
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
	int err = dx + dy, e2; /* error value e_xy */

	for (;;)
	{
		((uint32_t *) surface->pixels)[(y0 * surface->pitch) + x0] = color;

		if (x0 == x1 && y0 == y1)
			break;

		e2 = 2 * err;
		if (e2 >= dy)
		{	/* e_xy+e_x > 0 */
			err += dy; x0 += sx;
		}

		if (e2 <= dx)
		{	/* e_xy+e_y < 0 */
			err += dx; y0 += sy;
		}
	}
}

void renderer_fill_rect(int color, SDL_Rect rect)
{
	SDL_FillRect(surface, &rect, color);
}

void renderer_quit()
{
	DEBUG("destroying window");
	SDL_DestroyWindow(window);

	DEBUG("disabling SDL2");
	SDL_Quit();

	DEBUG("disabling SDL2 image subsystem");
	IMG_Quit();

	window = NULL;
	surface = NULL;
}

void renderer_draw_text_at(int x, int y, float scale, char *text)
{
	int size = 8;
	int curr_offset = x;
	int i;

	for (i = 0; i < strlen(text); i++)
	{
		char c = text[i];
		struct v2_8 offset = renderer_get_char_offset(c);

		renderer_draw_raw_texture(font_texture, &(SDL_Rect) {
			.x = offset.x * size,
			.y = offset.y * size,
			.w = 8,
			.h = 8
		}, &(SDL_Rect) {
			.x = curr_offset,
			.y = y,
			.w = 8 * scale,
			.h = 8 * scale
		});

		curr_offset += size * scale;
	}
}

static struct v2_8 renderer_get_char_offset(char c)
{
	uint8_t row = (c / 16) + 1;
	uint8_t col = (c - ((row - 1) * 16));

	return (struct v2_8) {
		.x = col,
		.y = row
	};
}
