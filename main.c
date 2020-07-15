#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include "behave.out.h"

#include "graphics.h"


unsigned int color_table[256];
int sps = 500000;
int placer_index = 1;
char placer_types[] = " icsfydt1x           ";

#define IN_SIZE 256
char in_buff[IN_SIZE];



void print_grid(grid_t grid) {
    iter(i, GRID_SIZE) {
        iter(j, GRID_SIZE) {
            cell_t* cell = grid[i][j];
            printf("%c ", cell->type);
        }
        printf("\n");
    }
}

void print_world(world_t world) {
    iter(m, WORLD_SIZE) { 
        iter(i, GRID_SIZE) { 
            iter(n, WORLD_SIZE) {
                iter(j, GRID_SIZE) {
                    printf("%c ", (*world[m][n])[i][j]->type);
                }
                printf("  ");
            }
            printf("\n");
        }
        printf("\n");
    }
}

void draw_world(world_t world) {
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            grid_t* grid = world[m][n];
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    unsigned int color =
                        (color_table[(*grid)[i][j]->type] & 0xffff0000) | ((*grid)[i][j]->data[0] << 8);
                    draw(n*GRID_SIZE+j, m*GRID_SIZE+i, color);
                }
            }
        }
    }
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void gamma_ray(world_t* world) {
    int n = rand() % WORLD_SIZE;
    int m = rand() % WORLD_SIZE;
    grid_t* grid = (*world)[m][n];
    int i = rand() % GRID_SIZE;
    int j = rand() % GRID_SIZE;
    cell_t* cell = (*grid)[i][j];

    int byte_select = rand() % (DATA_SIZE + 1);
    unsigned char mask = 1 << (rand() % 8);
    if (byte_select < DATA_SIZE) 
        cell->data[byte_select] ^= mask;
    else
        cell->type ^= mask;
}

int find(char* arr, char x, int size) {
    iter(i, size) {
        if (arr[i] == x)
            return i;
    }
    return -1;
}
    
void scan_for_strings(world_t* world) {
    char strings[256][IN_SIZE] = {0};
    char any[256] = {0};
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            grid_t* grid = (*world)[m][n];
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    cell_t* cell = (*grid)[i][j];
                    if (cell->type == 'i') {
                        unsigned char string_index = cell->data[0];
                        unsigned char c = cell->data[1];
                        unsigned char id = cell->data[2];
                        strings[id][string_index] = c;
                        any[id] = 1;
                    }
                }
            }
        }
    }

    iter(i, 256) {
        if (any[i]) {
            printf("%d: %s\n", i, strings[i]);
        }
    }
}


int poll_events(int* sps, world_t* world) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            close_window();
            return 1;
        }

        else if ((event.type == SDL_MOUSEBUTTONDOWN ||  event.type == SDL_MOUSEMOTION) 
                && event.button.button == SDL_BUTTON_LEFT) {
            int x = event.button.x / SCALE;
            int y = event.button.y / SCALE;
            //printf("%d %d\n", x, y);
            int i = y % GRID_SIZE;
            int j = x % GRID_SIZE;
            int m = y / GRID_SIZE;
            int n = x / GRID_SIZE;
            (*(*world)[m][n])[i][j]->type = placer_types[placer_index];
            memset((*(*world)[m][n])[i][j]->data, 0, DATA_SIZE);
        }
        else if ((event.type == SDL_MOUSEBUTTONDOWN ||  event.type == SDL_MOUSEMOTION) 
                && event.button.button == SDL_BUTTON_MIDDLE) {
            int x = event.button.x / SCALE;
            int y = event.button.y / SCALE;
            //printf("%d %d\n", x, y);
            int m = y / GRID_SIZE;
            int n = x / GRID_SIZE;
            iter(i, GRID_SIZE) {
                iter(j, GRID_SIZE) {
                    (*(*world)[m][n])[i][j]->type = placer_types[placer_index];
                    memset((*(*world)[m][n])[i][j]->data, 128, DATA_SIZE);
                }
            }
        }

        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym ) {
                case SDLK_q:
                case SDLK_ESCAPE:
                    close_window();
                    return 1;
                case SDLK_COMMA:
                    *sps *= 0.75;
                    printf("sps = %d\n", *sps);
                    break;
                case SDLK_PERIOD:
                    *sps /= 0.75;
                    printf("sps = %d\n", *sps);
                    break;
                case SDLK_1: placer_index = 1; break;
                case SDLK_2: placer_index = 2; break;
                case SDLK_3: placer_index = 3; break;
                case SDLK_4: placer_index = 4; break;
                case SDLK_5: placer_index = 5; break;
                case SDLK_6: placer_index = 6; break;
                case SDLK_7: placer_index = 7; break;
                case SDLK_8: placer_index = 8; break;
                case SDLK_9: placer_index = 9; break;
                case SDLK_0: placer_index = 0; break;
                case SDLK_g:
                    iter(i, 500) {
                        gamma_ray(world);
                    }
                    break;
                case SDLK_h: // scan all cells for strings
                    scan_for_strings(world);
                    break;
            }
        }
    }
    return 0;
}

int main() {
    // make read non-blocking
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    hole = malloc(sizeof(cell_t));
    hole->left = hole;
    hole->right = hole;
    hole->up = hole;
    hole->down = hole;

    srand(1234);
    iter(i, 256) {
        color_table[i] = rand() << 8;
    }
    //color_table[' '] = 0xffffff00;
    //color_table['1'] = 0x00ffff00;
    //color_table['d'] = 0xff00ff00;
    //color_table['x'] = 0xffff0000;
    //color_table['t'] = 0x00ff0000;
    srand(time(0)); 
    world_t world;

    // make 'em
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            grid_t* gridp = malloc(sizeof(grid_t));
            world[m][n] = gridp;
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    (*gridp)[i][j] = malloc(sizeof(cell_t));
                    memset((*gridp)[i][j]->data, 0, DATA_SIZE); // probably not needed
                    (*gridp)[i][j]->type = ' ';
                }
            }
        }
    }
    //(*world[1][1])[1][1]->type = '1';

    /*
    for (int i = -4; i < 5; i+=2) {
        int i_ = i + WORLD_SIZE/2;
        (*world[i_][WORLD_SIZE-1])[GRID_SIZE/2][GRID_SIZE/2]->type = 'd';
    }
    iter(i, 5) {
        (*world[WORLD_SIZE/2 - 5][WORLD_SIZE-1])[i][GRID_SIZE/2]->type = 'f';
        (*world[WORLD_SIZE/2 + 5][WORLD_SIZE-1])[i][GRID_SIZE/2]->type = 'f';
    }
    */
    //(*world[14][3])[1][1]->type = '1';

    /* The array is organized like this:
     * (*world[m][n])[i][j]->type = 'x';
     *
     *  n →
     *  m┌───────────────────────┐
     *  ↓│                       │
     *   │ ┌────┐ ┌────┐ ┌────┐  │
     *   │ │    │ │    │ │    │  │
     *   │ │    │ │    │ │    │  │
     *   │ └────┘ └────┘ └────┘  │
     *   │       j →             │
     *   │ ┌────┐i┌────┐ ┌────┐  │
     *   │ │    │↓│    │ │    │  │
     *   │ │    │ │    │ │    │  │
     *   │ └────┘ └────┘ └────┘  │
     *   │                       │
     *   │ ┌────┐ ┌────┐ ┌────┐  │
     *   │ │    │ │    │ │    │  │
     *   │ │    │ │    │ │    │  │
     *   │ └────┘ └────┘ └────┘  │
     *   └───────────────────────┘
     */

    // link em' up
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    // i down (positive)
                    if (i+1 < GRID_SIZE) {
                        (*world[m][n])[i][j]->down = 
                            (*world[m][n])[i+1][j];
                    } else if (m+1 < WORLD_SIZE) {
                        (*world[m][n])[i][j]->down = 
                            (*world[m+1][n])[0][j];
                    } else {
                        (*world[m][n])[i][j]->down = hole;
                    }

                    // i up (negative)
                    if (i-1 >= 0) {
                        (*world[m][n])[i][j]->up = 
                            (*world[m][n])[i-1][j];
                    } else if (m-1 >= 0) {
                        (*world[m][n])[i][j]->up = 
                            (*world[m-1][n])[GRID_SIZE-1][j];
                    } else {
                        (*world[m][n])[i][j]->up = hole;
                    }

                    // j right (positive)
                    if (j+1 < GRID_SIZE) {
                        (*world[m][n])[i][j]->right = 
                            (*world[m][n])[i][j+1];
                    } else if (n+1 < WORLD_SIZE) {
                        (*world[m][n])[i][j]->right = 
                            (*world[m][n+1])[i][0];
                    } else {
                        (*world[m][n])[i][j]->right = hole;
                    }

                    // j left (negative)
                    if (j-1 >= 0) {
                        (*world[m][n])[i][j]->left = 
                            (*world[m][n])[i][j-1];
                    } else if (n-1 >= 0) {
                        (*world[m][n])[i][j]->left = 
                            (*world[m][n-1])[i][GRID_SIZE-1];
                    } else {
                        (*world[m][n])[i][j]->left = hole;
                    }
                }
            }
        }
    }

    int foo = 0;
    int num_events = 0;
    int num_sites = WORLD_SIZE*WORLD_SIZE * GRID_SIZE*GRID_SIZE;
    graphics_init();
    clock_t last_time = clock();
    printf("      0123456789\n");
    printf("nums: %s\n", placer_types);

    while (1) {

        if (foo%sps == 0) {
            memset(in_buff, 0, IN_SIZE);
            int status = read(STDIN_FILENO, in_buff, IN_SIZE);
            unsigned char id = (rand() % 255) + 1;
            if (status != -1) {
                printf("Got: %s\n", in_buff);
                iter(i, IN_SIZE) {
                    if (in_buff[i] == '\0')
                        break;
                    cell_t* cell = (*world[0][0])[i%GRID_SIZE][i/GRID_SIZE];
                    cell->type = 'i';
                    cell->data[0] = i;
                    cell->data[1] = in_buff[i];
                    cell->data[2] = id;
                }
            }
            clock_t now_time = clock();
            clock_t time_diff = now_time - last_time;

            float seconds_passed = ((float) time_diff)/ CLOCKS_PER_SEC;
            //printf("      0123456789\n");
            //printf("nums: %s\n", placer_types);
            //printf("AER = %6.6f , FPS = %6.6f\n", ((float)num_events)/(seconds_passed * num_sites),
            //        1.0/seconds_passed);
            if (poll_events(&sps, &world))
                return 1;
            //print_world(world);
            draw_world(world);
            //printf("%d\n", foo);
            //usleep(1000*60);
            last_time = now_time;
            num_events = 0;
        }
        int n = rand() % WORLD_SIZE;
        int m = rand() % WORLD_SIZE;
        grid_t* grid = world[m][n];
        step(grid);

        foo++;
        num_events++;
    }
}


