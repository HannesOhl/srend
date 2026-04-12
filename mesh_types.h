#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include <stdint.h>

typedef struct {
	uint32_t v;
	uint32_t vt;
	uint32_t vn;
} Corner;

typedef struct {
	Corner c1;
	Corner c2;
	Corner c3;
} Face;

#endif

