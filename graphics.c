#include <pthread.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "constants.h"


SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture* buffer;
int pitch;
int* pixels;

int clamp(int x, int min, int max) {
    if (x < min)
        return min;
    else if (x > max)
        return max;
    return x;
}
void draw(int x, int y, unsigned int color1, unsigned int color2) {
    for (int i = 0; i < SCALE; i++) {
        for (int j = 0; j < SCALE; j++) {
            if (i < BORDER || i+BORDER >= SCALE || j < BORDER || j+BORDER >= SCALE)
                pixels[SCALE*x+i + (SCALE*y+j)*(WINDOW_WIDTH)] = color1;
            else
                pixels[SCALE*x+i + (SCALE*y+j)*(WINDOW_WIDTH)] = color2;
        }
    }
}


void graphics_init() {

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 2, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    buffer = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_BGRA8888,
            SDL_TEXTUREACCESS_STREAMING, 
            WINDOW_WIDTH,
            WINDOW_HEIGHT);

    pitch = 4*8 * WINDOW_WIDTH;
    pixels = malloc(sizeof(char)*4*WINDOW_WIDTH*WINDOW_HEIGHT);
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
    for (int i = 0; i < 100; i++){
        pixels[i+WINDOW_WIDTH*i] = 0xff000000;
    }
    
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
}

void render() {
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
}

void close_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

