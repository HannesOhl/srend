#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"

typedef struct {

	size_t pixels_number;

	uint32_t* buffer_frame;
	   float* buffer_depth;

	Camera camera;
} Renderer;

void renderer_init(Renderer* renderer, size_t pixels_number);
void buffer_depth_maximize(Renderer* renderer);

#endif

