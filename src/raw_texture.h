#ifndef RAW_TEXTURE_H
#define RAW_TEXTURE_H

#include <SDL2/SDL_render.h>

struct RawTexture
{
	int width, height;

	enum {
		FORMAT_XY,
		FORMAT_YX
	} format;

	unsigned *pixels;
};

struct RawTexture *raw_texture_from_surface(SDL_Surface *surface);
struct RawTexture *raw_texture_from_surface_slice(SDL_Surface *surface, int x, int y, int w, int h);

void raw_texture_destroy(struct RawTexture *self);

/** misc */
unsigned get_pixel_from_surface(SDL_Surface *surface, int x, int y);

#endif /** RAW_TEXTURE_H */
