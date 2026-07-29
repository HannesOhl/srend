#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "map.h"

typedef struct {
	bool active;
	bool grd;
	bool wfr;
	bool bfc;
	bool vfc;
	bool abb;
} Settings;

typedef struct {

	size_t pixels_number;

	uint32_t* buffer_frame;
	   float* buffer_depth;

	Camera camera;
	Settings settings;
} Renderer;

void renderer_init(Renderer* renderer, size_t pixels_number);
void buffer_depth_maximize(Renderer* renderer);
void settings_render(Renderer* renderer);
void map_render(Renderer* renderer, Map* map);
void grid_draw(uint32_t* buffer, Camera camera);

#endif

