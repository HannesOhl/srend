#include "../inc/backend_sdl.h"

#include <sanitizer/lsan_interface.h>

void context_sdl_init(SDLContext* ctx) {

	// disable lsan (to suppress SDL memory leak errors)
	__lsan_disable();

	SDL_Init(SDL_INIT_VIDEO);
	ctx->window = SDL_CreateWindow("SoftRend", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						   SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!ctx->window) fprintf(stderr, "Failed to create window. Error: %s\n", SDL_GetError());

	ctx->surface = SDL_GetWindowSurface(ctx->window);
	ctx->bytes_per_pixel = ctx->surface->format->BytesPerPixel;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	// re-enable lsan
	__lsan_enable();
}

void context_sdl_destroy(SDLContext* ctx) {

	SDL_DestroyWindow(ctx->window);
	free(ctx);
}

