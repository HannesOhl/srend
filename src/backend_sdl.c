#include "../inc/backend_sdl.h"

#ifdef DEBUG
#include <sanitizer/lsan_interface.h>
#endif

void context_sdl_init(SDLContext* ctx) {

	// disable lsan (to suppress SDL memory leak errors)
	#ifdef DEBUG
	__lsan_disable();
	#endif

	SDL_Init(SDL_INIT_VIDEO);
	ctx->window = SDL_CreateWindow("SoftRend", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						   SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!ctx->window) fprintf(stderr, "Failed to create window. Error: %s\n", SDL_GetError());

	ctx->surface = SDL_GetWindowSurface(ctx->window);
	ctx->bytes_per_pixel = ctx->surface->format->BytesPerPixel;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	// re-enable lsan
	#ifdef DEBUG
	__lsan_enable();
	#endif
}

void context_sdl_destroy(SDLContext* ctx) {

	SDL_DestroyWindow(ctx->window);
}

void input_handle(Renderer* renderer, Camera* c, bool* running) {

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	float mv_fac = 0.05f;

	// for camera
	V3f forward = c->forward;
	V3f right   = norm( cross(c->forward, c->up) );
	V3f up 	    = { .x = 0.0f, .y = 1.0f, .z = 0.0f };

	// camera movement keys
	if (keys[SDL_SCANCODE_E])      c->position = add(c->position, scale(mv_fac, forward));
	if (keys[SDL_SCANCODE_D])      c->position = sub(c->position, scale(mv_fac, forward));
	if (keys[SDL_SCANCODE_S])      c->position = sub(c->position, scale(mv_fac,   right));
	if (keys[SDL_SCANCODE_F])      c->position = add(c->position, scale(mv_fac,   right));
	if (keys[SDL_SCANCODE_SPACE])  c->position = add(c->position, scale(mv_fac,      up));
	if (keys[SDL_SCANCODE_LSHIFT]) c->position = sub(c->position, scale(mv_fac,      up));
	if (keys[SDL_SCANCODE_G])       printf("hi\n");

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
		// settings update
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) *running = false;
			if (event.key.keysym.sym == SDLK_TAB)     renderer->settings.active = !renderer->settings.active;
			if (event.key.keysym.sym == SDLK_g)       renderer->settings.grd    = !renderer->settings.grd;
			if (event.key.keysym.sym == SDLK_w)       renderer->settings.wfr    = !renderer->settings.wfr;
			if (event.key.keysym.sym == SDLK_b)       renderer->settings.bfc    = !renderer->settings.bfc;
			if (event.key.keysym.sym == SDLK_f)       renderer->settings.vfc    = !renderer->settings.vfc;
			if (event.key.keysym.sym == SDLK_c)       renderer->settings.abb    = !renderer->settings.abb;
			break;
		// camera movement mouse
		case SDL_MOUSEMOTION:
			V2f rel = { .x = (float) event.motion.xrel,
				    .y = (float) event.motion.yrel };
			camera_update_mouse(c, rel);
			break;
		default:
			break;
		}
	}
}


// for projectile later:
/*
	if (ctx->event.key.keysym.sym == SDLK_n) {
		rot2 = 0.0f;
		projectile.active = !projectile.active;
		projectile.pos = add(camera.position, scale(4.0f, camera.forward));
		projectile.vel = scale(0.5f, camera.forward);
		projectile.right = norm( cross(camera.forward, camera.up));
		projectile.up = (V3f) {{ 0.0f, 1.0f, 0.0f }};

	}

	if (projectile.active) {

			rot2 += 0.01f;
			V2f n = { .x = projectile.right.x, .y = projectile.right.z };
			n = norm(n);
			float rot = atan2f(n.x, n.y);

			model_render_advanced(&model[1], renderer,
					projectile.pos, projectile.up, rot, projectile.right, rot2);
			projectile.pos = add(projectile.pos, scale(0.1f, projectile.vel));
		}
*/

