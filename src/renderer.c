#include "../inc/renderer.h"

#include "../inc/text.h"

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

void settings_render(Renderer* renderer) {

	if (!renderer->settings.active) return;

	text_render(string_format(" w: wireframe         = %s\n", renderer->settings.wfr ? "on" : "off"),
		    0,  20, renderer->buffer_frame, GREEN, 2);
	text_render(string_format(" b: backface culling  = %s\n", renderer->settings.bfc ? "on" : "off"),
		    0,  40, renderer->buffer_frame, GREEN, 2);
	text_render(string_format(" f: frustum culling   = %s\n", renderer->settings.vfc ? "on" : "off"),
		    0,  60, renderer->buffer_frame, GREEN, 2);
	text_render(string_format(" c: show AABB         = %s\n", renderer->settings.abb ? "on" : "off"),
		    0,  80, renderer->buffer_frame, GREEN, 2);
	text_render(string_format(" g: show grid         = %s\n", renderer->settings.grd ? "on" : "off"),
		    0, 100, renderer->buffer_frame, GREEN, 2);
}

