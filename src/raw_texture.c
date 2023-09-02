#include "raw_texture.h"
#include "macros.h"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

unsigned get_pixel_from_surface(SDL_Surface *surface, int x, int y);

struct RawTexture *raw_texture_from_surface(SDL_Surface *surface)
{
	struct RawTexture *self;
	int i, j;

	ASSERT(surface != NULL, "invalid surface given");

	self = malloc(sizeof(struct RawTexture));

	self->pixels = malloc(sizeof(unsigned) * (surface->w * surface->h));
	self->width = surface->w;
	self->height = surface->h;

	for (i = 0; i < self->height; i++)
	{
		for (j = 0; j < self->width; j++)
		{
			uint8_t r, g, b, a;
			uint32_t pixel;

			pixel = get_pixel_from_surface(surface, j, i);
			SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			self->pixels[(i * self->width) + j] =	((r & 0xFF) << 24) |
								((g & 0xFF) << 16) |
								((b & 0xFF) <<  8) |
								(a & 0xFF);
		}
	}

	return self;
}

struct RawTexture *raw_texture_from_surface_slice(SDL_Surface *surface, int x, int y, int w, int h)
{
	struct RawTexture *self;
	int i, j;

	ASSERT(surface != NULL, "invalid surface given");

	self = malloc(sizeof(struct RawTexture));

	self->pixels = malloc(sizeof(unsigned) * (surface->w * surface->h));
	self->width = w;
	self->height = h;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			uint8_t r, g, b, a;
			uint32_t pixel;

			pixel = get_pixel_from_surface(surface, j + x, i + y);
			SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			self->pixels[(i * self->width) + j] =	((r & 0xFF) << 24) |
								((g & 0xFF) << 16) |
								((b & 0xFF) <<  8) |
								(a & 0xFF);
		}
	}

	return self;
}

void raw_texture_destroy(struct RawTexture *self)
{
	ASSERT(self != NULL);

	free(self->pixels);
	free(self);
}

/** https://stackoverflow.com/questions/53033971/how-to-get-the-color-of-a-specific-pixel-from-sdl-surface */
unsigned get_pixel_from_surface(SDL_Surface *surface, int x, int y)
{
	int bytes_per_pixel = surface->format->BytesPerPixel;

	uint8_t *pixel = (uint8_t *) surface->pixels + y * surface->pitch + x * bytes_per_pixel;

	switch (bytes_per_pixel)
	{
	case 1:
		return *pixel;
	case 2:
		return *(uint16_t *) pixel;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return pixel[0] << 16 | pixel[1] << 8 | pixel[2];
		else
			return pixel[2] << 16 | pixel[1] << 8 | pixel[0];
	case 4:
		return *(uint32_t *) pixel;
	default:
		ASSERT(false, "invalid bytes per pixel: %d", bytes_per_pixel);
	}

	return 0;
}
