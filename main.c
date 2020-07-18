#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include "behave.out.h"

#include "graphics.h"
#include "data_colors.h"

#define BLANK_CELL_TYPE 0
#define STRING_CELL_TYPE 1

//int sps = 500000;
int sps = (WORLD_SIZE * WORLD_SIZE * GRID_SIZE * GRID_SIZE * 60) / 60;
int placer_index = 1;

int last_clicked_m = 0;
int last_clicked_n = 0;

int data_select = 0;

#define IN_SIZE 256
char in_buff[IN_SIZE];

int brush_size = 1;


unsigned int repeat3(unsigned char x) {
    return (256-x) << 8 | x << 16 | x << 24;
}

//  rgb
// bgra
unsigned int rgb_to_bgra(unsigned int rgb) {
    return ((rgb & 0xff) << 24) | ((rgb & 0xff00) << 8) | ((rgb & 0xff0000) >> 8);
}

void draw_world(world_t world) {
    SDL_LockTexture(buffer, NULL, (void**) &pixels, &pitch);
    iter(m, WORLD_SIZE) { 
        iter(n, WORLD_SIZE) {
            grid_t* grid = world[m][n];
            iter(i, GRID_SIZE) { 
                iter(j, GRID_SIZE) {
                    unsigned char t = (*grid)[i][j]->type;
                    unsigned char d = (*grid)[i][j]->data[data_select];
                    unsigned int color1 = rgb_to_bgra(cell_colors[t]);
                    unsigned int color2 = t == BLANK_CELL_TYPE ? color1 : rgb_to_bgra(data_colors[d]);
                    draw(n*GRID_SIZE+j, m*GRID_SIZE+i, color1, color2);
                }
            }
        }
    }

    // draw menu bar
    iter(i, WORLD_SIZE*GRID_SIZE) {
        unsigned int color1 = rgb_to_bgra(cell_colors[i]);
        unsigned int color2 = i == placer_index ? 0xffffff00 : 0;
        draw(GRID_SIZE*WORLD_SIZE+1, i, color1, color2);
    }
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
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
                    if (cell->type == STRING_CELL_TYPE) {
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

void set_cell(world_t* world, int x, int y) {
    int i = y % GRID_SIZE;
    int j = x % GRID_SIZE;
    int m = y / GRID_SIZE;
    int n = x / GRID_SIZE;
    if (m >= WORLD_SIZE || n >= WORLD_SIZE || m < 0 || n < 0) {
        printf("tried to place outside of bounds\n");
        return;
    }
    (*(*world)[m][n])[i][j]->type = placer_index;
    memset((*(*world)[m][n])[i][j]->data, 0, DATA_SIZE);
}

cell_t* get_cell(world_t* world, int x, int y) {
    int i = y % GRID_SIZE;
    int j = x % GRID_SIZE;
    int m = (y / GRID_SIZE) % WORLD_SIZE;
    int n = (x / GRID_SIZE) % WORLD_SIZE;
    if (m >= WORLD_SIZE || n >= WORLD_SIZE || m < 0 || n < 0) {
        printf("tried to get cell outside of bounds\n");
        return hole;
    }
    if (i >= GRID_SIZE || j >= GRID_SIZE || i < 0 || j < 0) {
        printf("tried to get cell outside of grid\n");
        return hole;
    }
    return (*(*world)[m][n])[i][j];
}

void gamma_ray(world_t* world) {

    int x = rand() % (WORLD_SIZE * GRID_SIZE);
    int y = rand() % (WORLD_SIZE * GRID_SIZE);
    cell_t* cell = get_cell(world, x, y);

    int byte_select = rand() % (DATA_SIZE + 1);
    unsigned char mask = 1 << (rand() % 8);
    if (byte_select < DATA_SIZE) 
        cell->data[byte_select] ^= mask;
    else
        cell->type ^= mask;
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
            
            // click on menu  bar
            if (x >= GRID_SIZE*WORLD_SIZE) {
                placer_index = y;
                continue;
            }

            iter(i, brush_size) {
                iter(j, brush_size) {
                    set_cell(world, x+i-(brush_size/2), y+j-(brush_size/2));
                }
            }

            int m = y / GRID_SIZE;
            int n = x / GRID_SIZE;
            last_clicked_n = n; // used to choose where string are laid down
            last_clicked_m = m;
        }
        else if ((event.type == SDL_MOUSEBUTTONDOWN ||  event.type == SDL_MOUSEMOTION) 
                && event.button.button == SDL_BUTTON_MIDDLE) {
            int x = event.button.x / SCALE;
            int y = event.button.y / SCALE;
            int m = x / GRID_SIZE;
            int n = y / GRID_SIZE;

            iter(i, GRID_SIZE) {
                iter(j, GRID_SIZE) {
                    set_cell(world, m * GRID_SIZE + i, n * GRID_SIZE + j);
                }
            }
        }

        else if (event.type == SDL_KEYDOWN) {
            FILE *fp;
            switch (event.key.keysym.sym ) {
                case SDLK_q:
                case SDLK_ESCAPE:
                    close_window();
                    return 1;
                case SDLK_s:
                    fp = fopen("save.dat", "wb");  // create and/or overwrite
                    if (!fp) {
                        printf("Error in creating file. Aborting.\n");
                        break;
                    }
                    iter(x, GRID_SIZE * WORLD_SIZE) {
                        iter(y, GRID_SIZE * WORLD_SIZE) {
                            cell_t* cell = get_cell(world, x, y);
                            fwrite(&cell->type, sizeof(unsigned char), 1, fp);  
                            fwrite(&cell->data, sizeof(unsigned char), DATA_SIZE, fp);  
                        }
                    }
                    fclose(fp);
                    printf("Wrote out save file\n");
                    break;
                case SDLK_r:
                    fp = fopen("save.dat", "rb");  // read
                    if (!fp) {
                        printf("Error in opening file. Aborting.\n");
                        break;
                    }
                    iter(x, GRID_SIZE * WORLD_SIZE) {
                        iter(y, GRID_SIZE * WORLD_SIZE) {
                            cell_t* cell = get_cell(world, x, y);
                            fread(&cell->type, sizeof(unsigned char), 1, fp);
                            fread(&cell->data, sizeof(unsigned char), DATA_SIZE, fp);
                        }
                    }
                    fclose(fp);
                    printf("read in save file\n");
                    break;

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
                    iter(i, 1) {
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
                case SDLK_k:
                    data_select += 1;
                    data_select = data_select < 0 ? DATA_SIZE - 1 : data_select % DATA_SIZE;
                    printf("Data select: %d\n", data_select);
                    break;
                case SDLK_u:
                    brush_size -= 1;
                    if (brush_size <= 0) brush_size = 1;
                    printf("brush size = %d\n", brush_size);
                    break;
                case SDLK_i:
                    brush_size += 1;
                    printf("brush size = %d\n", brush_size);
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
            cell = (*world[last_clicked_m][last_clicked_n])[(i+x)%GRID_SIZE][(i+x)/GRID_SIZE];
            if (cell->type == BLANK_CELL_TYPE) {
                break;
            }
            x++;
        }
        cell->type = STRING_CELL_TYPE;
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
                    (*gridp)[i][j]->type = BLANK_CELL_TYPE;
                }
            }
        }
    }

    const char *tmp = "Hello world";
    memset(in_buff, 0, IN_SIZE);
    strncpy(in_buff, tmp, IN_SIZE - 1);
    print_in_buff_to_world(world);


    /* The array is organized like this:
     * (*world[m][n])[i][j]->type = whatever;
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


