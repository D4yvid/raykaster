#ifndef VEC_H
#define VEC_H

#include <math.h>
#include <stdint.h>

struct v2_8
{
	uint8_t x, y;
};

struct v2_16
{
	uint16_t x, y;
};

struct v2
{
	uint32_t x, y;
};

struct v2_64
{
	uint64_t x, y;
};


struct v2f
{
	float x, y;
};

struct v2f_64
{
	double x, y;
};

static float v2f_len(struct v2f v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

static float v2_len(struct v2 v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

#endif /** VEC_H */
