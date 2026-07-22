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

void input_handle(Camera* c, bool* running) {

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	float mv_fac = 0.05f;

	// camera movement
	V3f forward = c->forward;
	V3f right   = norm( cross(c->forward, c->up) );
	V3f up 	    = { .x = 0.0f, .y = 1.0f, .z = 0.0f };

	if (keys[SDL_SCANCODE_E])      c->position = add(c->position, scale(mv_fac, forward));
	if (keys[SDL_SCANCODE_D])      c->position = sub(c->position, scale(mv_fac, forward));
	if (keys[SDL_SCANCODE_S])      c->position = sub(c->position, scale(mv_fac,   right));
	if (keys[SDL_SCANCODE_F])      c->position = add(c->position, scale(mv_fac,   right));
	if (keys[SDL_SCANCODE_SPACE])  c->position = add(c->position, scale(mv_fac,      up));
	if (keys[SDL_SCANCODE_LSHIFT]) c->position = sub(c->position, scale(mv_fac,      up));

	// settings update
	if (keys[SDL_SCANCODE_ESCAPE]) *running = false;
	if (keys[SDL_SCANCODE_G])       state.grd = !state.grd;
	if (keys[SDL_SCANCODE_W])       state.wfr = !state.wfr;
	if (keys[SDL_SCANCODE_B])       state.bfc = !state.bfc;
	if (keys[SDL_SCANCODE_F])       state.vfc = !state.vfc;
	if (keys[SDL_SCANCODE_H])       state.hat = !state.hat;
	if (keys[SDL_SCANCODE_C])       state.abb = !state.abb;
}

