#include <stdio.h>
#include <limits.h>
#include <time.h>

#include "../inc/lalg.h"
#include "../inc/camera.h"
#include "../inc/text.h"
#include "../inc/color.h"
#include "../inc/backend_sdl.h"

#include "../assets/asset_cube.h"
#include "../assets/asset_teapot.h"
#include "../assets/asset_airboat.h"
#include "../assets/asset_humanoid_tri.h"

#define XMIN 40
#define YMIN 40
#define XMAX (SCREEN_WIDTH  - 40)
#define YMAX (SCREEN_HEIGHT - 40)

static const float EPS = 1e-8;

static         size_t lines_count_global    = 0;
static         size_t triangle_count_global = 0;

static float* buffer_depth = NULL;

typedef struct {
	uint32_t flags;
	bool grid_on;
	bool wireframe;
	bool bfc;
} State;

typedef struct {
	char* type;
	void* asset;
} MapObject;


State state = {
	.flags = 0,
	.grid_on = true,
	.wireframe = false,
	.bfc = false
};

void pixel_set(uint32_t x, uint32_t y, uint32_t* buffer, uint32_t color) {

	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
	buffer[x + y * SCREEN_WIDTH] = color;
}


static inline V3f world_to_view(V3f v_in, Camera c) {

	V3f res   = {0};
	V3f right = norm_3f(cross_3f(c.forward, c.up));

	V3f rel = {
		.x = v_in.x - c.position.x,
		.y = v_in.y - c.position.y,
		.z = v_in.z - c.position.z
	};

        res.x = rel.x * right.x     + rel.y * right.y     + rel.z * right.z;
        res.y = rel.x * c.up.x      + rel.y * c.up.y      + rel.z * c.up.z;
        res.z = rel.x * c.forward.x + rel.y * c.forward.y + rel.z * c.forward.z;

	return res;
}

V2s get_image_crd(V3f v, Camera camera) {

	const float znear = camera.znear;
	const float a = (float) SCREEN_HEIGHT / (float) SCREEN_WIDTH;
	const float f = 1 / tanf(0.5f * camera.fovy * M_PI / 180.0f);

	float px = a * f * v.x;
	float py = f * v.y;
	float pz = fmaxf(v.z, znear);

	// perspective divide
	px /= pz;
	py /= pz;

	int32_t x_screen = (int32_t)((px + 1.0f) * 0.5f * (int32_t)SCREEN_WIDTH);
	int32_t y_screen = (int32_t)((1.0f - py) * 0.5f * (int32_t)SCREEN_HEIGHT);

	// Clamp to avoid overflow of small V2s types
	if (x_screen < INT32_MIN) x_screen = INT32_MIN;
	if (x_screen > INT32_MAX) x_screen = INT32_MAX;
	if (y_screen < INT32_MIN) y_screen = INT32_MIN;
	if (y_screen > INT32_MAX) y_screen = INT32_MAX;

	return (V2s){ x_screen, y_screen };
}

// Liang–Barsky clipping.
bool clipline(int32_t *x1, int32_t *y1, int32_t *x2, int32_t *y2) {

	const float dx = (float) (*x2 - *x1);
	const float dy = (float) (*y2 - *y1);

	float p[4] = { -dx, dx, -dy, dy };
	float q[4] = { *x1 - (float) XMIN, (float) XMAX - *x1,
		       *y1 - (float) YMIN, (float) YMAX - *y1 };

	float u1 = 0.0f;
	float u2 = 1.0f;

	for (int i = 0; i < 4; ++i) {
		float pi = p[i];
		float qi = q[i];

		/* Handle near-parallel (pi == 0) robustly */
		if (fabs(pi) < EPS) {
			if (qi < 0.0f) return false;
		} else {
			float r = qi / pi;
	                if (pi < 0.0f) {
                		if (r > u1) u1 = r;
			} else {
		                if (r < u2) u2 = r;
           		}
			if (u1 > u2) return false;
            	}
	}

	/* compute clipped coordinates using original x1,y1,d */
	float nx1 = *x1 + u1 * dx;
	float ny1 = *y1 + u1 * dy;
	float nx2 = *x1 + u2 * dx;
	float ny2 = *y1 + u2 * dy;

	/* round to nearest integer (llround available in math.h) */
	*x1 = (int32_t) llroundf(nx1);
	*y1 = (int32_t) llroundf(ny1);
	*x2 = (int32_t) llroundf(nx2);
	*y2 = (int32_t) llroundf(ny2);

	return true;
}

void line_draw(V3f p1, V3f p2, uint32_t* buffer, uint32_t color, Camera camera) {

	// change to cam basis for near-plane clipping
	p1 = world_to_view(p1, camera);
	p2 = world_to_view(p2, camera);

	if (p1.z <= camera.znear && p2.z <= camera.znear) return;

	// this implies p1.z > c.znear
	if (p1.z < camera.znear) {
		// get new p0 behind clipping plane
		p1 = intersect_z(p2, p1, camera.znear);
	}

	// this implies p0.z > c.znear
	if (p2.z < camera.znear) {
		p2 = intersect_z(p1, p2, camera.znear);
	}

	V2s start = get_image_crd(p1, camera);
	V2s end   = get_image_crd(p2, camera);

	if ( start.x < (int32_t) XMIN && end.x < (int32_t) XMIN) return;
	if ( start.y < (int32_t) YMIN && end.y < (int32_t) YMIN) return;
	if ( start.x > (int32_t) XMAX && end.x > (int32_t) XMAX) return;
	if ( start.y > (int32_t) YMAX && end.y > (int32_t) YMAX) return;

	if (!clipline(&start.x, &start.y, &end.x, &end.y)) {
		return;
	}
	lines_count_global += 1;

	int32_t dx =  abs((int32_t)end.x - (int32_t)start.x);
	int32_t sx = (int32_t)start.x < (int32_t)end.x ? 1 : -1;

	int32_t dy = -abs((int32_t)end.y - (int32_t)start.y);
	int32_t sy = (int32_t)start.y < (int32_t)end.y ? 1 : -1;

	int32_t err = dx + dy;
	while (1) {
		pixel_set(start.x, start.y, buffer, color);
		if (start.x == end.x && start.y == end.y) break;
		int32_t e2 = 2 * err;
		if (e2 > dy) { err += dy; start.x += sx; }
		if (e2 < dx) { err += dx; start.y += sy; }
	}
}

void grid_draw(uint32_t* buffer, Camera camera) {

	int32_t grid_const = 40;
	uint32_t color = MAGENTA;

	for (int32_t i = -grid_const; i <= grid_const; i+=1) {
		V3f p1 = { .x = (float) i, .y = 0.0f, .z = -((float) grid_const) };
		V3f p2 = { .x = (float) i, .y = 0.0f, .z = +((float) grid_const) };
		line_draw(p1, p2, buffer, color, camera);
	}

	for (int32_t i = -grid_const; i <= grid_const; i+=1) {
		V3f p1 = { .x = -((float) grid_const), .y = 0.0f, .z = (float) i};
		V3f p2 = { .x = +((float) grid_const), .y = 0.0f, .z = (float) i};
		line_draw(p1, p2, buffer, color, camera);
	}
}

void buffer_flush(uint32_t* buffer, uint8_t bytes_per_pixel) {

	memset(buffer, 0, PIXELS_NUMBER * bytes_per_pixel);
}

void background_render(uint32_t* buffer) {
	for (size_t x = XMIN; x < XMAX; x++) {
	for (size_t y = YMIN; y < YMAX; y++) {
		buffer[x + y*SCREEN_WIDTH] = 0;
	}}
}
static inline Color shade_color(Color c, float intensity)
{
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;

    uint32_t i = (uint32_t)(intensity * 255.0f);

    uint32_t r = ((c >> 24) & 0xFF) * i >> 8;
    uint32_t g = ((c >> 16) & 0xFF) * i >> 8;
    uint32_t b = ((c >>  8) & 0xFF) * i >> 8;
    uint32_t a =  (c        & 0xFF);

    return (r << 24) | (g << 16) | (b << 8) | a;
}

void triangle_draw(Triangle t, uint32_t* buffer, Camera camera, Color color) {

	if (state.wireframe) {
		line_draw(t.v1, t.v2, buffer, color, camera);
		line_draw(t.v1, t.v3, buffer, color, camera);
		line_draw(t.v2, t.v3, buffer, color, camera);
		return;
	}

	// change to cam basis (view space)
	t.v1 = world_to_view(t.v1, camera);
	t.v2 = world_to_view(t.v2, camera);
	t.v3 = world_to_view(t.v3, camera);

	float znear = camera.znear;
	if (t.v1.z <= znear && t.v2.z <= znear && t.v3.z <= znear) return;

	// also for lighting
	V3f n = cross_3f( sub_3f(t.v2, t.v1), sub_3f(t.v3, t.v1) );
	if (state.bfc) {
		// cull backfaces
		if ( dot_3f(t.v1, n) <= 0 ) return;
	}

	V2s v1 = get_image_crd(t.v1, camera);
	V2s v2 = get_image_crd(t.v2, camera);
	V2s v3 = get_image_crd(t.v3, camera);

	// skip trangles that are completely out of the image
	if ( v1.x < (int32_t) XMIN && v2.x < (int32_t) XMIN && v3.x < (int32_t) XMIN) return;
	if ( v1.y < (int32_t) YMIN && v2.y < (int32_t) YMIN && v3.y < (int32_t) YMIN) return;
	if ( v1.x > (int32_t) XMAX && v2.x > (int32_t) XMAX && v3.x > (int32_t) XMAX) return;
	if ( v1.y > (int32_t) YMAX && v2.y > (int32_t) YMAX && v3.y > (int32_t) YMAX) return;

	// bounding box
	int32_t x_min = v1.x;
	if (v2.x < x_min) x_min = v2.x;
	if (v3.x < x_min) x_min = v3.x;
	int32_t x_max = v1.x;
	if (v2.x > x_max) x_max = v2.x;
	if (v3.x > x_max) x_max = v3.x;
	int32_t y_min = v1.y;
	if (v2.y < y_min) y_min = v2.y;
	if (v3.y < y_min) y_min = v3.y;
	int32_t y_max = v1.y;
	if (v2.y > y_max) y_max = v2.y;
	if (v3.y > y_max) y_max = v3.y;

	// adjust bounding box to the images clipping space
	if (x_min < (int32_t) XMIN) x_min = (int32_t) XMIN;
	if (x_max > (int32_t) XMAX) x_max = (int32_t) XMAX;
	if (y_min < (int32_t) YMIN) y_min = (int32_t) YMIN;
	if (y_max > (int32_t) YMAX) y_max = (int32_t) YMAX;

	// precompute floats for screen coords
	float v1x = (float)v1.x, v1y = (float)v1.y;
	float v2x = (float)v2.x, v2y = (float)v2.y;
	float v3x = (float)v3.x, v3y = (float)v3.y;
	float iz1 = 1.0f / t.v1.z;
	float iz2 = 1.0f / t.v2.z;
	float iz3 = 1.0f / t.v3.z;

	float denom = (v2y - v3y)*(v1x - v3x) + (v3x - v2x)*(v1y - v3y);
	if (fabsf(denom) < EPS) return;

	// get shaded color for triangle based on global light pos
	n = norm_3f(n);
	V3f light_global_pos_w = { .x = 5.0f, .y = 5.0f, .z = 5.0f };
	V3f light_global_pos_v = world_to_view(light_global_pos_w, camera);
	V3f dir = norm_3f(sub_3f(light_global_pos_v, t.v1));
	float intensity = dot_3f(dir, n);
	if (intensity < 0.0f) intensity = 0.0f;
	color = shade_color(color, intensity);

	float invDenom = 1.0f / denom;

	for (int32_t y = y_min; y <= y_max; ++y) {
	for (int32_t x = x_min; x <= x_max; ++x) {

		// sample at pixel center
		float px = (float) x + 0.5f;
		float py = (float) y + 0.5f;

		// barycentric coords
		float a = ( (v2y - v3y) * (px - v3x) + (v3x - v2x) * (py - v3y) ) * invDenom;
		float b = ( (v3y - v1y) * (px - v3x) + (v1x - v3x) * (py - v3y) ) * invDenom;
		float c = 1.0f - a - b;

		// TODO: this is a hack, we need a consistent top-left rule
		const float BC_EPS = 1e-6f;
		if (a < -BC_EPS || b < -BC_EPS || c < -BC_EPS) continue;

		// get z for depth buffer
		float z_inv = a * iz1 + b * iz2 + c * iz3;
		if (fabsf(z_inv) < EPS) continue;
		float z = 1.0f / z_inv;

		size_t idx = (size_t) x + (size_t) y * (size_t) SCREEN_WIDTH;
		if (z < buffer_depth[idx]) {
			buffer_depth[idx] = z;
			pixel_set(x, y, buffer, color);
		}
	}}
    	triangle_count_global += 1;
}

void time_measure_start(struct timespec* t0) {
	clock_gettime(CLOCK_MONOTONIC, t0);
}

double time_measure_end_ms(struct timespec* t1, const struct timespec* t0) {
	clock_gettime(CLOCK_MONOTONIC, t1);

	time_t sec  = t1->tv_sec  - t0->tv_sec;
	long   nsec = t1->tv_nsec - t0->tv_nsec;

	if (nsec < 0) {
		sec-=1;
		nsec += 1000000000L;
	}

	return sec * 1000.0 + nsec / 1e6;
}

void event_loop(SDLContext* ctx, uint32_t* buffer, Camera camera) {

	// for fps calculation
	struct timespec t0 = {0};
	struct timespec t1 = {0};

	bool running = true;

	Triangle tri1 = {
		.v1 = {{ +0.0f, +0.0f, +0.0f }},
		.v2 = {{ +1.0f, +0.0f, +0.0f }},
		.v3 = {{ +0.5f, +1.0f, +0.0f }}
	};

	Triangle tri2 = {
		.v1 = {{ +0.0f, +0.0f, +1.0f }},
		.v2 = {{ +1.0f, +0.0f, +1.0f }},
		.v3 = {{ +0.5f, +1.0f, +1.0f }}
	};

	size_t samples_cur      = 0;
	double t_ms_avg          = 0;
	double t_ms_avg_rolling = 0;
	while (running) {

		time_measure_start(&t0);
		while (SDL_PollEvent(&ctx->event) != 0) {

			switch (ctx->event.type) {

			case SDL_KEYDOWN:
				if (ctx->event.key.keysym.sym == SDLK_ESCAPE) running = false;
				if (ctx->event.key.keysym.sym == SDLK_g) state.grid_on = !state.grid_on;
				if (ctx->event.key.keysym.sym == SDLK_w) state.wireframe = !state.wireframe;
				if (ctx->event.key.keysym.sym == SDLK_b) state.bfc = !state.bfc;
				break;

			case SDL_MOUSEMOTION:
				V2f rel = {
					.x = ctx->event.motion.xrel,
					.y = ctx->event.motion.yrel,
				};
				camera_update_mouse(&camera, rel);

			default:
				break;
			}
		}

		const Uint8* keys = SDL_GetKeyboardState(NULL);
		float mv_fac = 0.05f;

		if (keys[SDL_SCANCODE_E]) {
			camera.position = add_3f(camera.position,
			scal_3f(mv_fac, camera.forward));
		}

		if (keys[SDL_SCANCODE_D]) {
			V3f dir = norm_3f((V3f) {{ camera.forward.x, 0.0f, camera.forward.z }} );
			camera.position = sub_3f(camera.position,
			scal_3f(mv_fac, dir));
		}

		if (keys[SDL_SCANCODE_S]) {
			V3f dir = norm_3f(cross_3f(camera.up, camera.forward));
			camera.position = add_3f(camera.position,
			scal_3f(mv_fac, dir));
		}

		if (keys[SDL_SCANCODE_F]) {
			V3f dir = norm_3f(cross_3f(camera.forward, camera.up));
			camera.position = add_3f(camera.position,
			scal_3f(mv_fac, dir));
		}

		if (keys[SDL_SCANCODE_SPACE]) {
			camera.position = add_3f(camera.position,
			scal_3f(mv_fac, (V3f) {{ 0.0f, 1.0f, 0.0f }} ));
		}

		if (keys[SDL_SCANCODE_LSHIFT]) {
			camera.position = add_3f(camera.position,
			scal_3f(mv_fac, (V3f) {{ 0.0f, -1.0f, 0.0f }} ));
		}

		if (state.grid_on) grid_draw(buffer, camera);
/*
		for (size_t i = 0; i < asset_cube.f_count; i++) {
			Triangle t = {
				.v1 = asset_cube.v[asset_cube.f[i].x-1],
				.v2 = asset_cube.v[asset_cube.f[i].y-1],
				.v3 = asset_cube.v[asset_cube.f[i].z-1]
			};

			triangle_draw(t, buffer, camera, GREEN);
		}
*/
		static float angle = 0.0f;
		angle += 0.01;
		M3f mat_rot1 = rot_xz_3f(+angle);
		M3f mat_rot2 = rot_xz_3f(-angle);

		for (int i = asset_teapot.f_count - 1; i >= 0; i--) {
			Triangle t = {
				.v1 = mul_m3f_v3f(mat_rot1, asset_teapot.v[asset_teapot.f[i].x-1]),
				.v2 = mul_m3f_v3f(mat_rot1, asset_teapot.v[asset_teapot.f[i].y-1]),
				.v3 = mul_m3f_v3f(mat_rot1, asset_teapot.v[asset_teapot.f[i].z-1])
			};
			triangle_draw(t, buffer, camera, EMERALD);
		}

		for (int i = asset_teapot.f_count - 1; i >= 0; i--) {

			V3f v1 = {
				.x = asset_teapot.v[asset_teapot.f[i].x-1].x,
				.y = asset_teapot.v[asset_teapot.f[i].x-1].y,
				.z = asset_teapot.v[asset_teapot.f[i].x-1].z + 10
			};

			V3f v2 = {
				.x = asset_teapot.v[asset_teapot.f[i].y-1].x,
				.y = asset_teapot.v[asset_teapot.f[i].y-1].y,
				.z = asset_teapot.v[asset_teapot.f[i].y-1].z + 10
			};

			V3f v3 = {
				.x = asset_teapot.v[asset_teapot.f[i].z-1].x,
				.y = asset_teapot.v[asset_teapot.f[i].z-1].y,
				.z = asset_teapot.v[asset_teapot.f[i].z-1].z + 10
			};

			Triangle t = {
				.v1 =mul_m3f_v3f(mat_rot2, v1),
				.v2 =mul_m3f_v3f(mat_rot2, v2),
				.v3 =mul_m3f_v3f(mat_rot2, v3)
			};
			triangle_draw(t, buffer, camera, ROUGE);
		}

		//triangle_draw(tri1, buffer, camera, EMERALD);
		//triangle_draw(tri2, buffer, camera, ROUGE);
		//V3f origin = {{8.0f, 0.0f, 8.0f}};
		//cube_draw(origin, 2.0f, buffer, RED, camera);

		for (size_t i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
			buffer_depth[i] = FLT_MAX;
		}
		SDL_UpdateWindowSurface(ctx->window);
		buffer_flush(buffer, ctx->bytes_per_pixel);
		background_render(buffer);

		// end time measuring
		t_ms_avg_rolling += time_measure_end_ms(&t1, &t0);
		samples_cur += 1;
		if (samples_cur == 256) {
			t_ms_avg = t_ms_avg_rolling / 256.0;
			t_ms_avg_rolling = 0;
			samples_cur = 0;
		}

		text_render(string_format("frame time = %.2f ms, FPS = %.2f (256 samples), "
					  "lines drawn = %zu, triangles drawn = %zu\n",
					t_ms_avg, 1000.0 / t_ms_avg, lines_count_global,
					triangle_count_global), 0, 0, buffer, GREEN, 2);

		text_render(string_format("wireframe = %s\n", state.wireframe ? "on" : "off"), 0, 20, buffer, GREEN, 2);
		text_render(string_format("backface culling = %s\n", state.bfc ? "on" : "off"), 0, 40, buffer, GREEN, 2);

		lines_count_global     = 0;
		triangle_count_global = 0;
	}
}

int main(void) {

	buffer_depth = calloc((size_t) SCREEN_WIDTH*SCREEN_HEIGHT,
			      (size_t) sizeof *buffer_depth);
	for (size_t i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
		buffer_depth[i] = FLT_MAX;
	}

	SDLContext* ctx = calloc((size_t) 1, (size_t) sizeof *ctx);

	context_sdl_init(ctx);

	uint32_t* buffer = ctx->surface->pixels;

	Camera camera;
	camera_default_set(&camera);

	event_loop(ctx, buffer, camera);

	context_sdl_destroy(ctx);
	free(buffer_depth);
	SDL_Quit();

	return 0;
}

