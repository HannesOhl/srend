#include "../inc/renderer.h"

#include "../inc/text.h"

#include <stdlib.h>
#include <float.h>

static const float EPS = 1e-8f;
static const uint32_t SCREEN_WIDTH  = 1200;
static const uint32_t SCREEN_HEIGHT = 700;

#define XMIN 40
#define YMIN 40
#define XMAX (SCREEN_WIDTH  - 40)
#define YMAX (SCREEN_HEIGHT - 40)

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

static void pixel_set(int32_t x, int32_t y, uint32_t* buffer, uint32_t color) {

	if (x >= (int32_t) SCREEN_WIDTH || y >= (int32_t) SCREEN_HEIGHT) return;
	buffer[x + y * (int32_t) SCREEN_WIDTH] = color;
}

static V3f world_to_view(V3f v_in, Camera c) {

	V3f res   = {0};
	V3f right = norm(cross(c.forward, c.up));

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

static V2s get_image_crd(V3f v, Camera camera) {

	const float znear = camera.znear;
	const float a = (float) SCREEN_HEIGHT / (float) SCREEN_WIDTH;
	const float f = 1.0f / tanf(0.5f * camera.fovy * (float) M_PI / 180.0f);

	float px = a * f * v.x;
	float py = f * v.y;
	float pz = fmaxf(v.z, znear);

	// perspective divide
	px /= pz;
	py /= pz;

	int32_t x_screen = (int32_t) ( (px + 1.0f) * 0.5f * (float) SCREEN_WIDTH  );
	int32_t y_screen = (int32_t) ( (1.0f - py) * 0.5f * (float) SCREEN_HEIGHT );

	// clamp to avoid overflow of small V2s types
	if (x_screen < INT32_MIN) x_screen = INT32_MIN;
	if (x_screen > INT32_MAX) x_screen = INT32_MAX;
	if (y_screen < INT32_MIN) y_screen = INT32_MIN;
	if (y_screen > INT32_MAX) y_screen = INT32_MAX;

	return (V2s) { x_screen, y_screen };
}

// Liang–Barsky clipping.
static bool clipline(int32_t *x1, int32_t *y1, int32_t *x2, int32_t *y2) {

	const float dx = (float) (*x2 - *x1);
	const float dy = (float) (*y2 - *y1);

	float p[4] = { -dx, dx, -dy, dy };
	float q[4] = { (float) *x1 - (float) XMIN, (float) XMAX - (float) *x1,
		       (float) *y1 - (float) YMIN, (float) YMAX - (float) *y1 };

	float u1 = 0.0f;
	float u2 = 1.0f;

	for (int i = 0; i < 4; ++i) {
		float pi = p[i];
		float qi = q[i];

		// handle near-parallel (pi == 0)
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

	float nx1 = (float) *x1 + u1 * dx;
	float ny1 = (float) *y1 + u1 * dy;
	float nx2 = (float) *x1 + u2 * dx;
	float ny2 = (float) *y1 + u2 * dy;

	*x1 = (int32_t) llroundf(nx1);
	*y1 = (int32_t) llroundf(ny1);
	*x2 = (int32_t) llroundf(nx2);
	*y2 = (int32_t) llroundf(ny2);

	return true;
}

static void line_draw(V3f p1, V3f p2, uint32_t* buffer, uint32_t color, Camera camera) {

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
	//lines_count_global += 1;

	int32_t dx =  abs( (int32_t) end.x - (int32_t) start.x );
	int32_t sx = (int32_t) start.x < (int32_t) end.x ? 1 : -1;

	int32_t dy = -abs( (int32_t) end.y - (int32_t) start.y );
	int32_t sy = (int32_t) start.y < (int32_t) end.y ? 1 : -1;

	int32_t err = dx + dy;
	while (1) {
		pixel_set(start.x, start.y, buffer, color);
		if (start.x == end.x && start.y == end.y) break;
		int32_t e2 = 2 * err;
		if (e2 > dy) { err += dy; start.x += sx; }
		if (e2 < dx) { err += dx; start.y += sy; }
	}
}

static void triangle_draw(Triangle t, V2f uv1, V2f uv2, V2f uv3, Renderer* renderer, Color color, const Texture* tex) {

	Camera camera = renderer->camera;
	uint32_t* buffer = renderer->buffer_frame;
	Settings state = renderer->settings;

	if (state.wfr) {
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
	V3f n = norm(cross( sub(t.v2, t.v1), sub(t.v3, t.v1) ));
	if (state.bfc) {
		// cull backfaces
		if ( dot(t.v1, n) <= 0 ) return;
	}

	V2s v1 = get_image_crd(t.v1, camera);
	V2s v2 = get_image_crd(t.v2, camera);
	V2s v3 = get_image_crd(t.v3, camera);

	// skip triangles that are completely out of the image
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
	float v1x = (float) v1.x, v1y = (float) v1.y;
	float v2x = (float) v2.x, v2y = (float) v2.y;
	float v3x = (float) v3.x, v3y = (float) v3.y;
	float iz1 = 1.0f / t.v1.z;
	float iz2 = 1.0f / t.v2.z;
	float iz3 = 1.0f / t.v3.z;

	float denom = (v2y - v3y)*(v1x - v3x) + (v3x - v2x)*(v1y - v3y);
	if (fabsf(denom) < EPS) return;

	float denom_inv = 1.0f / denom;
	const float BC_EPS = 1e-6f;

	uint32_t tw = tex->width;
	uint32_t th = tex->height;

	for (int32_t y = y_min; y <= y_max; ++y) {
		float py = (float) y + 0.5f;
		for (int32_t x = x_min; x <= x_max; ++x) {

			// sample at pixel center
			float px = (float) x + 0.5f;

			// barycentric coords
			float px_minus_v3x = px - v3x;
			float py_minus_v3y = py - v3y;
			float a = ( (v2y - v3y) * px_minus_v3x + (v3x - v2x) * py_minus_v3y ) * denom_inv;
			float b = ( (v3y - v1y) * px_minus_v3x + (v1x - v3x) * py_minus_v3y ) * denom_inv;
			float c = 1.0f - a - b;

			// TODO: this is a hack, we need a consistent top-left rule
			if (a < -BC_EPS || b < -BC_EPS || c < -BC_EPS) continue;

			// get z for depth buffer
			float z_inv = 1.0f / (a * iz1 + b * iz2 + c * iz3);
			if (fabsf(z_inv) < EPS) continue;
			float z = 1.0f * z_inv;

			float u_over_z = a * (uv1.x * iz1) + b * (uv2.x * iz2) + c * (uv3.x * iz3);
			float v_over_z = a * (uv1.y * iz1) + b * (uv2.y * iz2) + c * (uv3.y * iz3);

			float u = u_over_z * z_inv;
			float v = v_over_z * z_inv;

			uint32_t tx = (uint32_t) (u * (float) (tw - 1));
			uint32_t ty = (uint32_t) ((1.0f - v ) * (float) (th - 1));

			if (tx >= tw) tx = tw - 1;
			if (ty >= th) ty = th - 1;

			size_t idx = (tx + ty*tw) * 4;
			uint32_t current_pixel =
				    ((uint32_t) tex->pixels[idx + 0] << 16) |
				    ((uint32_t) tex->pixels[idx + 1] << 8)  |
				    ((uint32_t) tex->pixels[idx + 2] << 0)  |
				    ((uint32_t) tex->pixels[idx + 3] << 24);
			Color texel = current_pixel;

			idx = (size_t) x + (size_t) y * (size_t) SCREEN_WIDTH;
			if (z < renderer->buffer_depth[idx]) {
				renderer->buffer_depth[idx] = z;
				pixel_set(x, y, buffer, texel);
			}
		}
	}
    	//triangle_count_global += 1;
}


static void model_render(Model model, Renderer* renderer, V3f offset) {

	for (size_t i = 0; i < model.mesh->f_count; i++) {
		Triangle t = {
			.v1 = add(model.mesh->v[model.mesh->f[i].c1.v-1], offset),
			.v2 = add(model.mesh->v[model.mesh->f[i].c2.v-1], offset),
			.v3 = add(model.mesh->v[model.mesh->f[i].c3.v-1], offset)
		};

		V2f vt1 = model.mesh->vt[model.mesh->f[i].c1.vt-1];
		V2f vt2 = model.mesh->vt[model.mesh->f[i].c2.vt-1];
		V2f vt3 = model.mesh->vt[model.mesh->f[i].c3.vt-1];

		triangle_draw(t, vt1, vt2, vt3, renderer, GREEN, model.tex);
	}
}

void map_render(Renderer* renderer, Map* map) {

	for (size_t i = 0; i < map->entity_number; i++) {
		model_render(*map->entity[i].model, renderer, map->entity[i].pos);
	}
}

void grid_draw(uint32_t* buffer, Camera camera) {

	int32_t grid_const = 40;
	uint32_t color = MAGENTA;

	for (int32_t i = -grid_const; i <= grid_const; i+=1) {
		V3f p1 = { .x = (float) i, .y = 0.0f, .z = -( (float) grid_const ) };
		V3f p2 = { .x = (float) i, .y = 0.0f, .z = +( (float) grid_const ) };
		line_draw(p1, p2, buffer, color, camera);
	}

	for (int32_t i = -grid_const; i <= grid_const; i+=1) {
		V3f p1 = { .x = -( (float) grid_const ), .y = 0.0f, .z = (float) i};
		V3f p2 = { .x = +( (float) grid_const ), .y = 0.0f, .z = (float) i};
		line_draw(p1, p2, buffer, color, camera);
	}
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

