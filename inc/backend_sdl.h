#ifndef BACKEND_SDL_H
#define BACKEND_SDL_H

#include "../inc/SDL2/include/SDL.h"

static const uint32_t SCREEN_WIDTH  = 1200;
static const uint32_t SCREEN_HEIGHT = 700;
static const uint32_t PIXELS_NUMBER = SCREEN_WIDTH*SCREEN_HEIGHT;

typedef struct {
	SDL_Window*  window;
	SDL_Surface* surface;
	SDL_Event    event;
	uint32_t     bytes_per_pixel;
} SDLContext;

void context_sdl_init(SDLContext* ctx);
void context_sdl_destroy(SDLContext* ctx);

#endif

