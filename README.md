### Overview
This is a small software-renderer/game-engine i wrote to better understand rendering pipelines and the basics of computer graphics.

We use a minimal set of dependencies. SDL is only used to handle user input and to open a window. We then have access to the pixel buffer:

```
SDL_Surface surface = SDL_GetWindowSurface(window);
uint32_t* pixel_buffer = surface->pixels;
```
We also make use of C23's feature #embed, therefore GCC version 15 is used. 

### How to run
This project is a personal project, thus no guarantees are made. Currently no release branch is provided. It might compile, it might not. 

### Additional Credits 
All the blender models, if not explicitly credited, were done by a friend of mine. Thanks!
