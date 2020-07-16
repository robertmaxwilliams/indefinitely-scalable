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
//int sps = 500000;
int sps = (WORLD_SIZE * WORLD_SIZE * GRID_SIZE * GRID_SIZE * 60) / 60;
int placer_index = 1;
char placer_types[] = " isdrwm           ";
int data_select = 0;

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

unsigned int repeat3(unsigned char x) {
    return (256-x) << 8 | x << 16 | x << 24;
}

void draw_world(world_t world) {
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            grid_t* grid = world[m][n];
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    unsigned char t = (*grid)[i][j]->type;
                    unsigned int color1 = (color_table[t]);
                    unsigned int color2 = t == ' ' ? color1 : repeat3((*grid)[i][j]->data[data_select]);
                    draw(n*GRID_SIZE+j, m*GRID_SIZE+i, color1, color2);
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
                    if (cell->type == 's') {
                        unsigned char id = cell->data[0];
                        unsigned char string_index = cell->data[1];
                        unsigned char c = cell->data[2];
                        strings[id][string_index] = c;
                        any[id] = 1;
                    }
                }
            }
        }
    }

    printf("\nStrings:\n");
    iter(i, 256) {
        if (any[i]) {
            // replace \0 with ? when inside string 
            int is_in_string = 0;
            for (int j = IN_SIZE-1; j >= 0; j--) {
                if (is_in_string && strings[i][j] == '\0')
                    strings[i][j] = '?';
                else if (strings[i][j] != '\0')
                    is_in_string = 1;
            }
            strings[i][IN_SIZE-1] = '\0';
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
                    memset((*(*world)[m][n])[i][j]->data, 0, DATA_SIZE);
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
                    *sps /= 10;
                    if (*sps < 16)
                        *sps = 16;
                    printf("sps = %d\n", *sps);
                    break;
                case SDLK_PERIOD:
                    *sps *= 10;
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
                case SDLK_j: // scan all cells for strings
                    data_select -= 1;
                    data_select = data_select < 0 ? DATA_SIZE - 1 : data_select % DATA_SIZE;
                    printf("Data select: %d\n", data_select);
                    break;
                case SDLK_k: // scan all cells for strings
                    data_select += 1;
                    data_select = data_select < 0 ? DATA_SIZE - 1 : data_select % DATA_SIZE;
                    printf("Data select: %d\n", data_select);
                    break;
            }
        }
    }
    return 0;
}

void print_in_buff_to_world(world_t world) {
    unsigned char id = (rand() % 255) + 1;
    int x = 0;
    for (int i = 0; i < IN_SIZE; i++) {
        if (in_buff[i] == '\0')
            return;
        cell_t* cell;

        // find next empty square
        while (1) {
            // if we've taken up more than one grid, quit
            if (i+x >= GRID_SIZE * GRID_SIZE)
                return;
            cell = (*world[0][0])[(i+x)%GRID_SIZE][(i+x)/GRID_SIZE];
            if (cell->type == ' ') {
                break;
            }
            x++;
        }
        cell->type = 's';
        cell->data[0] = id;
        cell->data[1] = i;
        cell->data[2] = in_buff[i];
    }
}

void update_world(world_t world, int steps) {
    iter(i, steps) {
        int n = rand() % WORLD_SIZE;
        int m = rand() % WORLD_SIZE;
        grid_t* grid = world[m][n];
        step(grid);
    }
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

    srand(1234590);
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
    const char *tmp = "Hello world";
    memset(in_buff, 0, IN_SIZE);
    strncpy(in_buff, tmp, IN_SIZE - 1);
    print_in_buff_to_world(world);

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

    double num_sites = WORLD_SIZE*WORLD_SIZE * GRID_SIZE*GRID_SIZE;
    graphics_init();
    printf("      0123456789\n");
    printf("nums: %s\n", placer_types);

    while (1) {

        // physics steps
        clock_t update_start = clock();
        update_world(world, sps);
        double seconds_to_update = ((double) clock() - update_start)/ CLOCKS_PER_SEC;

        // drawing step
        clock_t render_start = clock();
        draw_world(world);
        double seconds_to_render = ((double) clock() - render_start)/ CLOCKS_PER_SEC;

        double seconds_to_wait = (1.0/60.0) - seconds_to_render - seconds_to_update;
        if (seconds_to_wait < 0)
            seconds_to_wait = 0;

        // terminal keyboard input
        SDL_Delay(seconds_to_wait/1000);
        memset(in_buff, 0, IN_SIZE);
        int status = read(STDIN_FILENO, in_buff, IN_SIZE);
        if (status != -1) {
            if (strlen(in_buff) > 1) {
                in_buff[strlen(in_buff)-1] = '\0';
                printf("Got: %s\n", in_buff);
                print_in_buff_to_world(world);
            } else {
                scan_for_strings(&world);
                double seconds_passed = seconds_to_render + seconds_to_update + seconds_to_wait;
                printf("AER = %6.6f , FPS = %6.6f\n", ((double)sps)/(seconds_passed * num_sites),
                        1.0/seconds_passed);
                printf("render = %6.6f ms, physics = %6.6f ms, wait = %6.6f ms\n", seconds_to_render*1000, 
                        seconds_to_update*1000, seconds_to_wait*1000);
            }

        }

        // X keyboard and mouse input
        if (poll_events(&sps, &world))
            return 1;
    }
}


