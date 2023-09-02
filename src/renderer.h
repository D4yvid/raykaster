#ifndef RENDERER_H
#define RENDERER_H

#include "raw_texture.h"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <stdint.h>

#define FRAMEBUFFER_WIDTH	500
#define FRAMEBUFFER_HEIGHT	300
#define FRAMEBUFFER_PIXELSCALE	2

union Color {
	struct {
		uint8_t a, b, g, r;
	};

	int color;
};

void renderer_init();

void renderer_set_pallete(SDL_Color *pallete, int start, int count);

void renderer_draw_line(int color, int x1, int y1, int x2, int y2);
void renderer_fill_rect(int color, SDL_Rect rect);

void renderer_draw_raw_texture(struct RawTexture *texture, SDL_Rect *slice, SDL_Rect *pos);
void renderer_draw_surface(SDL_Surface *_surface, SDL_Rect *slice, SDL_Rect *pos);

void renderer_clear_screen();
void renderer_draw_screen();

void renderer_draw_text_at(int x, int y, float scale, char *text);

void renderer_quit();

#endif /** RENDERER_H */
