#include "game.h"
#include "macros.h"
#include "map.h"
#include "raw_texture.h"
#include "renderer.h"
#include "texture.h"
#include "vec.h"
#include "font.h"
#include "textures.h"
#include "pallete.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void game_render_ui();
static void game_render_scene();

unsigned framebuffer_pixels[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_WIDTH];
struct RawTexture framebuffer = {
	.width = FRAMEBUFFER_WIDTH,
	.height = FRAMEBUFFER_HEIGHT,
	.format = FORMAT_XY,
	.pixels = framebuffer_pixels
};

static float screen_x, persp_wall_dist, wall_x, texture_x, texture_y, texture_step, texture_pos;
static int hit, side, line_height, line_start, line_end, texture_index, face;
static struct v2 map_pos, step;
static struct v2f ray_direction, side_dist, delta_dist;

static struct v2f ray_direction_0, ray_direction_1, tb_step, tb_pos;
static struct v2 current_cell, tb_texture_pos;
static int p;
static float pos_z, row_distance;

static int x, y, nx, ny, val, valid;
static union Color texture_color, temp_color;

void game_render()
{
	game_render_scene();
	game_render_ui();

	memset(framebuffer.pixels, 0xFF222222, sizeof(unsigned) * (framebuffer.width * framebuffer.height));
}

static void game_render_scene()
{
	/** Ceiling & floor casting */

	for (y = 0; y < FRAMEBUFFER_HEIGHT; y++)
	{
		ray_direction_0.x = camera.dir.x - camera.plane.x;
		ray_direction_0.y = camera.dir.y - camera.plane.y;
		ray_direction_1.x = camera.dir.x + camera.plane.x;
		ray_direction_1.y = camera.dir.y + camera.plane.y;

		p = y - FRAMEBUFFER_HEIGHT / 2;
		pos_z = 0.5 * FRAMEBUFFER_HEIGHT;

		row_distance = pos_z / p;

		tb_step.x = row_distance * (ray_direction_1.x - ray_direction_0.x) / FRAMEBUFFER_WIDTH;
		tb_step.y = row_distance * (ray_direction_1.y - ray_direction_0.y) / FRAMEBUFFER_WIDTH;

		tb_pos.x = camera.pos.x + row_distance * ray_direction_0.x;
		tb_pos.y = camera.pos.y + row_distance * ray_direction_0.y;

		for (x = 0; x < FRAMEBUFFER_WIDTH; x++)
		{
			current_cell.x = (int) tb_pos.x;
			current_cell.y = (int) tb_pos.y;

			tb_texture_pos.x = (int) (TEXTURE_WIDTH * (tb_pos.x - current_cell.x)) & (TEXTURE_WIDTH - 1);
			tb_texture_pos.y = (int) (TEXTURE_HEIGHT * (tb_pos.y - current_cell.y)) & (TEXTURE_HEIGHT - 1);

			tb_pos.x += tb_step.x;
			tb_pos.y += tb_step.y;

			texture_color.color = 0x2596beFF;

			framebuffer.pixels[((FRAMEBUFFER_HEIGHT - y - 1) * FRAMEBUFFER_WIDTH) + x] = texture_color.color;

			if (
				(current_cell.x > MAP_WIDTH - 1 || current_cell.x <= 0) ||
				(current_cell.y > MAP_HEIGHT - 1 || current_cell.y <= 0)
			) continue;

			val = abs(MAP[current_cell.x][current_cell.y]);

			if (val > TEXTURE_COUNT)
				val = TEXTURE_COUNT;

			if (val <= 0)
				val = 3;

			/** Floor */
			temp_color.color = textures[val - 1]->pixels[(tb_texture_pos.y * TEXTURE_WIDTH) + tb_texture_pos.x];

			if (val != 3)
			{
				temp_color.r /= row_distance > 1.5 ? row_distance * 1.29700 : 1.96666666;
				temp_color.g /= row_distance > 1.5 ? row_distance * 1.29700 : 1.96666666;
				temp_color.b /= row_distance > 1.5 ? row_distance * 1.29700 : 1.96666666;
			}

			texture_color = temp_color;

			framebuffer.pixels[(y * FRAMEBUFFER_WIDTH) + x] = texture_color.color;
		}
	}

	/** Scene ray casting */

	for (x = 0; x < FRAMEBUFFER_WIDTH; x++)
	{
		/* This will make the center of the screen have the coordinate 0,
		 * the left at -1 and right at 1 */
		screen_x = 2 * x / (float) FRAMEBUFFER_WIDTH - 1.f;

		ray_direction.x = camera.dir.x + camera.plane.x * screen_x;
		ray_direction.y = camera.dir.y + camera.plane.y * screen_x;

		hit = 0, side = 0;

		map_pos.x = camera.pos.x;
		map_pos.y = camera.pos.y;

		delta_dist.x = ray_direction.x == 0 ? 1e30 : fabs(1 / ray_direction.x);
		delta_dist.y = ray_direction.x == 0 ? 1e30 : fabs(1 / ray_direction.y);

		if (ray_direction.x < 0)
		{
			step.x = -1;
			side_dist.x = (camera.pos.x - map_pos.x) * delta_dist.x;
		}
		else {
			step.x = 1;
			side_dist.x = (map_pos.x + 1 - camera.pos.x) * delta_dist.x;
		}

		if (ray_direction.y < 0)
		{
			step.y = -1;
			side_dist.y = (camera.pos.y - map_pos.y) * delta_dist.y;
		}
		else {
			step.y = 1;
			side_dist.y = (map_pos.y + 1 - camera.pos.y) * delta_dist.y;
		}

		while (hit <= 0)
		{
			if (side_dist.x < side_dist.y)
			{
				side_dist.x += delta_dist.x;
				map_pos.x += step.x;
				side = 0;
			}
			else {
				side_dist.y += delta_dist.y;
				map_pos.y += step.y;
				side = 1;
			}

			if ((map_pos.x >= MAP_WIDTH || map_pos.x < 0) || (map_pos.y >= MAP_HEIGHT || map_pos.y < 0))
			{
				goto _next_scanline;
			}

			hit = MAP[map_pos.x][map_pos.y];
		}

		if (side == 0)	persp_wall_dist = (side_dist.x - delta_dist.x);
		else		persp_wall_dist = (side_dist.y - delta_dist.y);

		line_height = (float) (FRAMEBUFFER_HEIGHT / persp_wall_dist);
		line_start  = (-line_height / 2) + (FRAMEBUFFER_HEIGHT / 2);
		line_end    = (line_height / 2) + (FRAMEBUFFER_HEIGHT / 2);

		if (line_start < 0) line_start = 0;
		if (line_end >= FRAMEBUFFER_HEIGHT) line_end = FRAMEBUFFER_HEIGHT;

		texture_index = MAP[map_pos.x][map_pos.y] - 1;
		
		if (side == 0)	wall_x = camera.pos.y + persp_wall_dist * ray_direction.y;
		else		wall_x = camera.pos.x + persp_wall_dist * ray_direction.x;

		/** Determine which face was hit */
		if (side == 0) {
			if (ray_direction.x < 0) {
				face = 1;
			} else {
				face = 2;
			}
		} else {
			if (ray_direction.y < 0) {
				face = 3;
			} else {
				face = 4;
			}
		}

		wall_x -= floor(wall_x);

		texture_x = (int) (wall_x * (double) TEXTURE_WIDTH);

		if (
			(side == 0 && ray_direction.x > 0) ||
			(side == 1 && ray_direction.y < 0)
		) texture_x = TEXTURE_WIDTH - texture_x - 1;

		texture_step = 1.f * TEXTURE_HEIGHT / line_height;
		texture_pos = (line_start - FRAMEBUFFER_HEIGHT / 2.f + line_height / 2.f) * texture_step;

		nx = map_pos.x, ny = map_pos.y, valid = false;

		switch (face)
		{
		case 1:
			nx = map_pos.x - side + 1;
			break;
		case 2:
			nx = map_pos.x + side - 1;
			break;
		case 3:
			ny = map_pos.y + side;
			break;
		case 4:
			ny = map_pos.y - side;
			break;
		}

		if (!(nx < 0 || nx > MAP_WIDTH) || !(ny < 0 || ny > MAP_HEIGHT)) {
			valid = true;
		}

		for (y = line_start; y < line_end; y++)
		{
			texture_y = (int) texture_pos & (TEXTURE_HEIGHT - 1);
			texture_pos += texture_step;

			texture_color.color = 0x2596beFF;

			if (texture_index > 0)
			{
				texture_color.color = textures[texture_index - 1]->pixels[(int) ((texture_y * TEXTURE_WIDTH) + texture_x)];

				if (valid && MAP[nx][ny] < 0) {
					texture_color.r /= persp_wall_dist > 1.5 ? persp_wall_dist * 1.29700 : 1.96666666;
					texture_color.g /= persp_wall_dist > 1.5 ? persp_wall_dist * 1.29700 : 1.96666666;
					texture_color.b /= persp_wall_dist > 1.5 ? persp_wall_dist * 1.29700 : 1.96666666;
				}
			}

			framebuffer.pixels[(y * framebuffer.width) + x] = texture_color.color;
		}

_next_scanline: {}
	}

	renderer_draw_raw_texture(&framebuffer, NULL, NULL);
}

static void game_render_ui()
{
	char buffer[512];

	snprintf(buffer, sizeof(buffer), "FPS: %.0f", ceil(1000.f / delta_time));

	renderer_draw_text_at(2, 4, 3, buffer);

	DEBUG_ONLY({
		float deg;

		deg = atan2(camera.dir.y, camera.dir.x) * 180 / M_PI;

		snprintf(
			buffer,
			sizeof(buffer),
			"X: %.2f Z: %.2f  A: %.2f (D: %.2f, %.2f | P: %.2f %.2f)",
			camera.pos.x,
			camera.pos.y,
			deg,
			camera.dir.x,
			camera.dir.y,
			camera.plane.x,
			camera.plane.y
		);

		renderer_draw_text_at(2, 32, 1, buffer);
	});
}

