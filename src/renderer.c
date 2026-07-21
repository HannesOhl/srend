#include "../inc/renderer.h"

void renderer_init(Renderer* renderer, size_t pixels_number) {

	// alloc and init frame buffer to 0
	renderer->buffer_frame = calloc((size_t) pixels_number,
					(size_t) sizeof *renderer->buffer_frame);

	// alloc and init depth buffer to FLT_MAX
	renderer->buffer_depth = calloc((size_t) pixels_number,
			                (size_t) sizeof *renderer->buffer_depth);

	for (size_t i = 0; i < pixels_number; i++) {
		renderer->buffer_depth[i] = FLT_MAX;
	}
}

