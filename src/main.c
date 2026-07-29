// TODO: use model up/right instead of projectile/player
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <arpa/inet.h>

#define UNIT_TEST
#include "../tools/lalg/lalg.h"

#include "../inc/renderer.h"
#include "../inc/camera.h"
#include "../inc/text.h"
#include "../inc/color.h"
#include "../inc/backend_sdl.h"
#include "../inc/textures.h"

#include "../inc/meshes/mesh_player.h"
#include "../inc/meshes/mesh_messa.h"

#define MAX_MODEL_NUM 5
Model model[MAX_MODEL_NUM];

Map map00 = {
	.entity_number = 2,
	.entity[0] = {
		&model[0],
		{{0.0f, 0.0f, 0.0f}}
	},
	.entity[1] = {
		&model[0],
		{{5.0f, 0.0f, 5.0f}}
	}
};

Model model_assemble(const void* asset, const Texture* asset_texture) {

	const uint8_t* a = (const uint8_t*) asset;
	Mesh* mesh = calloc((size_t) 1, sizeof *mesh);

	// copy counts to model and set pointers to correct mesh data
	size_t offset = 0;
	memcpy(&mesh->v_count,  a + offset, sizeof mesh->v_count);
	offset += sizeof mesh->v_count;
	memcpy(&mesh->vt_count, a + offset, sizeof mesh->vt_count);
	offset += sizeof mesh->vt_count;
	memcpy(&mesh->vn_count, a + offset, sizeof mesh->vn_count);
	offset += sizeof mesh->vn_count;
	memcpy(&mesh->f_count,  a + offset, sizeof mesh->f_count);
	offset += sizeof mesh->f_count;

	mesh->v  = (const V3f*)  (a + offset);
	offset  += sizeof *mesh->v  * mesh->v_count;
	mesh->vt = (const V2f*)  (a + offset);
	offset  += sizeof *mesh->vt * mesh->vt_count;
	mesh->vn = (const V3f*)  (a + offset);
	offset  += sizeof *mesh->vn * mesh->vn_count;
	mesh->f  = (const Face*) (a + offset);
	offset  += sizeof *mesh->f  * mesh->f_count;

	Model res = {
		.mesh = mesh,
		.tex = asset_texture
	};

	// calculate initial bounding box
	float x_min = +FLT_MAX; float y_min = +FLT_MAX; float z_min = +FLT_MAX;
	float x_max = -FLT_MAX; float y_max = -FLT_MAX; float z_max = -FLT_MAX;
	for (size_t i = 0; i < res.mesh->v_count; i++) {

		float x = res.mesh->v[i].x;
		float y = res.mesh->v[i].y;
		float z = res.mesh->v[i].z;

		if (x < x_min) x_min = x;
		if (x > x_max) x_max = x;
		if (y < y_min) y_min = y;
		if (y > y_max) y_max = y;
		if (z < z_min) z_min = z;
		if (z > z_max) z_max = z;
	}

	res.bb.center = (V3f) {{ (x_min+x_max)/2.0f, (y_min+y_max)/2.0f, (z_min+z_max)/2.0f }};
	res.bb.extend = (V3f) {{ (x_max-x_min)/2.0f, (y_max-y_min)/2.0f, (z_max-z_min)/2.0f }};

	return res;
}

static size_t lines_count_global	   = 0;
static size_t triangle_count_global	   = 0;
static size_t models_rejected_count_global = 0;

typedef struct {
	V3f position;
	V3f forward;
} Player;
Player player = {
	.position = { .x = 0.0f, .y = 0.0f, .z = 0.0f },
	.forward  = { .x = 0.0f, .y = 0.0f, .z = 1.0f }
};

void player_info() {
	float x = player.position.x;
	float y = player.position.y;
	float z = player.position.z;

	printf("Player Info: \n");
	printf("r(x, y, z) = (%6.2f, %6.2f, %6.2f)\n", x, y, z);
}

void buffer_flush(uint32_t* buffer, uint32_t bytes_per_pixel) {

	memset(buffer, 0, PIXELS_NUMBER * bytes_per_pixel);
}
/*
void background_render(uint32_t* buffer) {
	for (size_t x = XMIN; x < XMAX; x++) {
	for (size_t y = YMIN; y < YMAX; y++) {
		buffer[x + y*SCREEN_WIDTH] = 0;
	}}
}

void aabb_render(BoundingBox bb, uint32_t* buffer, Camera camera) {

	float min_x = sub(bb.center, bb.extend).x;
	float min_y = sub(bb.center, bb.extend).y;
	float min_z = sub(bb.center, bb.extend).z;
	float max_x = add(bb.center, bb.extend).x;
	float max_y = add(bb.center, bb.extend).y;
	float max_z = add(bb.center, bb.extend).z;

	V3f min_ground_min = { .x = min_x, .y = min_y, .z = min_z};
	V3f min_ground_max = { .x = min_x, .y = min_y, .z = max_z};
	V3f max_ground_min = { .x = max_x, .y = min_y, .z = min_z};
	V3f max_ground_max = { .x = max_x, .y = min_y, .z = max_z};

	V3f min_sky_min = { .x = min_x, .y = max_y, .z = min_z};
	V3f min_sky_max = { .x = min_x, .y = max_y, .z = max_z};
	V3f max_sky_min = { .x = max_x, .y = max_y, .z = min_z};
	V3f max_sky_max = { .x = max_x, .y = max_y, .z = max_z};

	line_draw(min_ground_min, min_ground_max, buffer, GREEN, camera);
	line_draw(min_ground_min, max_ground_min, buffer, GREEN, camera);
	line_draw(max_ground_min, max_ground_max, buffer, GREEN, camera);
	line_draw(min_ground_max, max_ground_max, buffer, GREEN, camera);

	line_draw(min_sky_min, min_sky_max, buffer, GREEN, camera);
	line_draw(min_sky_min, max_sky_min, buffer, GREEN, camera);
	line_draw(max_sky_min, max_sky_max, buffer, GREEN, camera);
	line_draw(min_sky_max, max_sky_max, buffer, GREEN, camera);

	line_draw(min_ground_min, min_sky_min, buffer, GREEN, camera);
	line_draw(min_ground_max, min_sky_max, buffer, GREEN, camera);
	line_draw(max_ground_min, max_sky_min, buffer, GREEN, camera);
	line_draw(max_ground_max, max_sky_max, buffer, GREEN, camera);
}

void model_render_advanced(Model* model, Renderer* renderer,
		V3f offset, V3f ax1, float r, V3f ax2, float r2) {

	Camera camera = renderer->camera;
	uint32_t* buffer = renderer->buffer_frame;
	Settings state = renderer->settings;

	for (size_t i = 0; i < model->mesh->f_count; i++) {
		Triangle t = {
			.v1 = add(rot_ax(rot_ax(model->mesh->v[model->mesh->f[i].c1.v-1], ax1, r), ax2, r2), offset),
			.v2 = add(rot_ax(rot_ax(model->mesh->v[model->mesh->f[i].c2.v-1], ax1, r), ax2, r2), offset),
			.v3 = add(rot_ax(rot_ax(model->mesh->v[model->mesh->f[i].c3.v-1], ax1, r), ax2, r2), offset)
		};

		V2f vt1 = model->mesh->vt[model->mesh->f[i].c1.vt-1];
		V2f vt2 = model->mesh->vt[model->mesh->f[i].c2.vt-1];
		V2f vt3 = model->mesh->vt[model->mesh->f[i].c3.vt-1];

		triangle_draw(t, vt1, vt2, vt3, renderer, GREEN, model->tex);
	}

	BoundingBox new = {
		.center = add(model->bb.center, offset),
		.extend = model->bb.extend
	};
	if (state.abb) aabb_render(new, buffer, camera);
}
*/
Triangle triangle_offset(Triangle t, V3f offset) {
	Triangle res = {0};

	res.v1 = add(t.v1, offset);
	res.v2 = add(t.v2, offset);
	res.v3 = add(t.v3, offset);

	return res;
}

typedef struct {
	size_t samples_number;

	struct timespec t0;
	struct timespec t1;

	size_t samples_cur;
	double t_ms_avg;
	double t_ms_avg_rolling;
} Timer;

void time_measure_start(Timer* tmr) {
	clock_gettime(CLOCK_MONOTONIC, &tmr->t0);
}

void time_measure_end(Timer* tmr) {

	clock_gettime(CLOCK_MONOTONIC, &tmr->t1);

	time_t sec  = tmr->t1.tv_sec  - tmr->t0.tv_sec;
	long   nsec = tmr->t1.tv_nsec - tmr->t0.tv_nsec;

	if (nsec < 0) {
		 sec -= 1;
		nsec += 1000000000L;
	}

	tmr->samples_cur += 1;
	if (tmr->samples_cur == 128) {
		tmr->t_ms_avg = tmr->t_ms_avg_rolling / 128.0;
		tmr->t_ms_avg_rolling = 0;
		tmr->samples_cur = 0;
	}

	tmr->t_ms_avg_rolling += (double) sec * 1000.0 + (double) nsec / 1e6;
}

void frame_info_print(Timer* tmr, uint32_t* buffer) {

	text_render(string_format(" frame time = %.2f ms, FPS = %.2f (128 samples), "
				  "lines drawn = %zu, triangles drawn = %zu\n",
				tmr->t_ms_avg, 1000.0 / tmr->t_ms_avg, lines_count_global,
				triangle_count_global), 0, 0, buffer, GREEN, 2);
}

void event_loop(SDLContext* ctx, Renderer* renderer, Model* model) {


	Camera camera = renderer->camera;
	uint32_t* buffer = renderer->buffer_frame;

	// to calculate fps and frame time
	Timer tmr = {0};

	bool running = true;
	while (running) {

		time_measure_start(&tmr);
		input_handle(renderer, &camera, &running);

		if (renderer->settings.grd) grid_draw(buffer, camera);

		map_render(renderer, &map00);

		settings_render(renderer);

		SDL_UpdateWindowSurface(ctx->window);
		buffer_flush(buffer, ctx->bytes_per_pixel);
		//background_render(buffer);

		// reset depth buffer
		buffer_depth_maximize(renderer);

		// end time measuring
		time_measure_end(&tmr);

		//
		frame_info_print(&tmr, buffer);

		lines_count_global     	     = 0;
		triangle_count_global 	     = 0;
		models_rejected_count_global = 0;

		renderer->camera = camera;
	}
}

int main(void) {

	#ifdef UNIT_TEST
		if (!LALG_tests_run()) {
			fprintf(stderr, "lalg tests failed!\n");
			exit(-2);
		}
	#endif

	Renderer renderer = {0};
	renderer_init(&renderer, SCREEN_WIDTH * SCREEN_HEIGHT);

	SDLContext ctx = {0};
	context_sdl_init(&ctx);

	renderer.buffer_frame = ctx.surface->pixels;

	// prepare models
	model[0] = model_assemble(&mesh_player  , &texture_player);
	model[1] = model_assemble(&mesh_messa, &texture_messa);
	//model[1] = model_assemble(&asset_zylinder, &texture_zylinder);
	//model[2] = model_assemble(&asset_zaubererhut, &texture_zaubererhut);
	//model[4] = model_assemble(&mesh_lok, &texture_lok);

	// event loop
	event_loop(&ctx, &renderer, model);

	// clean-up
	free(model[0].mesh);
	context_sdl_destroy(&ctx);
	SDL_Quit();

	return 0;
}

