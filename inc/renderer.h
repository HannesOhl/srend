#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"

typedef struct {
	uint32_t* buffer_frame;
	uint32_t* buffer_depth;

	Camera camera;
} Renderer;

void renderer_init(Renderer* renderer);

#endif

