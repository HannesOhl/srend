#include "../inc/renderer.h"

#include <stdlib.h>
#include <float.h>

void buffer_depth_maximize(Renderer* renderer) {

	for (size_t i = 0; i < renderer->pixels_number; i++) {
		renderer->buffer_depth[i] = FLT_MAX;
	}
}

void renderer_init(Renderer* renderer, size_t pixels_number) {

	// set number of pixels for renderer
	renderer->pixels_number = pixels_number;

	// alloc and init frame buffer to 0
	renderer->buffer_frame = calloc((size_t) pixels_number,
					(size_t) sizeof *renderer->buffer_frame);

	// alloc and init depth buffer to FLT_MAX
	renderer->buffer_depth = calloc((size_t) pixels_number,
			                (size_t) sizeof *renderer->buffer_depth);
	buffer_depth_maximize(renderer);

	// init camera
	camera_init(&renderer->camera);

	// init settings
	renderer->settings.grd = true;
}

