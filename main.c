#include <stdio.h>

// needed for reading and writing and non-blocking read
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "constants.h"
#include "behave.out.h"
#include "graphics.h"
#include "data_colors.h"

#define BLANK_CELL_TYPE 0
#define STRING_CELL_TYPE 1

// start out at about 60 AER
int sps = (WORLD_SIZE * WORLD_SIZE * 60) / 60;
int placer_index = 1;

int last_clicked_x = 0;
int last_clicked_y = 0;

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
    iter(x, WORLD_SIZE) {
        iter(y, WORLD_SIZE) {
            unsigned char t = world[x][y]->type;
            unsigned char d = world[x][y]->data[data_select];
            unsigned int color1 = rgb_to_bgra(cell_colors[t]);
            unsigned int color2 = t == BLANK_CELL_TYPE ? color1 : rgb_to_bgra(data_colors[d]);
            draw(x, y, color1, color2);
        }
    }

    // draw menu bar
    iter(column, NUM_SIDEBAR_COLUMNS) {
        iter(row, NUM_SIDEBAR_ROWS) {
            unsigned int color1 = rgb_to_bgra(cell_colors[row+ column*NUM_SIDEBAR_ROWS]);
            unsigned int color2 = (row+column*NUM_SIDEBAR_ROWS) == placer_index ? 0xffffff00 : 0;
            draw_sidebar(row, column, color1, color2);
        }
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
    
void scan_for_strings(world_t world) {
    char strings[256][IN_SIZE] = {0};
    char any[256] = {0};
    iter(x, WORLD_SIZE) { 
        iter(y, WORLD_SIZE) {
            cell_t* cell = world[x][y];
            if (cell->type == STRING_CELL_TYPE) {
                unsigned char id = cell->data[0];
                unsigned char string_index = cell->data[1];
                unsigned char c = cell->data[2];
                strings[id][string_index] = c;
                any[id] = 1;
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

void set_cell(world_t world, int x, int y) {
    if (x >= WORLD_SIZE || y >= WORLD_SIZE || x < 0 || y < 0) {
        printf("tried to place outside of bounds\n");
        return;
    }
    world[x][y]->type = placer_index;
    memset(world[x][y]->data, 0, DATA_SIZE);
}

cell_t* get_cell(world_t world, int x, int y) {
    if (x >= WORLD_SIZE || y >= WORLD_SIZE || x < 0 || y < 0) {
        printf("tried to place outside of bounds\n");
        return hole;
    }
    return world[x][y];
}

void gamma_ray(world_t world) {

    int x = rand() % WORLD_SIZE;
    int y = rand() % WORLD_SIZE;
    cell_t* cell = get_cell(world, x, y);

    int byte_select = rand() % (DATA_SIZE + 1);
    unsigned char mask = 1 << (rand() % 8);
    // initially I was doing bit flips, but byte randomization is more uniform
    // that is, bit flips cause certain types and states to be more likely than they should
    // based on bit distance and optimizing for that is too much trouble.
    // In the future, a greycode layout of the types woudld resolve this in a nice way.
    if (byte_select < DATA_SIZE) 
        cell->data[byte_select] = rand() % 256;
    else
        cell->type = rand() % 256;
}

void print_cell(cell_t* cell) {
    printf("type: %3d, data = [", cell->type);
    iter (i, 10) {
        printf("%3d, ", cell->data[i]);
    }
    printf("]\n");
}


int poll_events(int* sps, world_t world) {
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
            if (x >= WORLD_SIZE) {
                placer_index = click_sidebar(event.button.x, event.button.y);
                continue;
            }

            iter(i, brush_size) {
                iter(j, brush_size) {
                    set_cell(world, x+i-(brush_size/2), y+j-(brush_size/2));
                }
            }

            last_clicked_x = x; // used to choose where string are laid down
            last_clicked_y = y;
        }
        else if ((event.type == SDL_MOUSEBUTTONDOWN ||  event.type == SDL_MOUSEMOTION) 
                && event.button.button == SDL_BUTTON_MIDDLE) {
            int x = event.button.x / SCALE;
            int y = event.button.y / SCALE;

            iter(i, 16) {
                iter(j, 16) {
                    set_cell(world, x+i, y+j);
                }
            }
        }

        else if ((event.type == SDL_MOUSEBUTTONDOWN ||  event.type == SDL_MOUSEMOTION) 
                && event.button.button == SDL_BUTTON_RIGHT) {
            int x = event.button.x / SCALE;
            int y = event.button.y / SCALE;
            print_cell(get_cell(world, x, y));
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
                    iter(x, WORLD_SIZE) {
                        iter(y, WORLD_SIZE) {
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
                    iter(x, WORLD_SIZE) {
                        iter(y, WORLD_SIZE) {
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
                    if (*sps < 1)
                        *sps = 1;
                    printf("sps = %d\n", *sps);
                    break;
                case SDLK_PERIOD:
                    if (*sps <= 1)
                        *sps = 16;
                    else
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
            if (i+x >= 256)
                return;
            cell = get_cell(world, last_clicked_x + (i + x)%16, last_clicked_y + (i+x)/16);
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
        int x = rand() % WORLD_SIZE;
        int y = rand() % WORLD_SIZE;
        step(world, x, y);
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
    iter(x, WORLD_SIZE) { 
        iter(y, WORLD_SIZE) {
            world[x][y] = malloc(sizeof(cell_t));
            memset(world[x][y]->data, 0, DATA_SIZE); // probably not needed
            world[x][y]->type = BLANK_CELL_TYPE;
        }
    }

    const char *tmp = "Hello world";
    memset(in_buff, 0, IN_SIZE);
    strncpy(in_buff, tmp, IN_SIZE - 1);
    print_in_buff_to_world(world);


    /* The array is organized like this:
     * world[x][y] = whatever;
     *
     *  x →    
     *  y┌────┐
     *  ↓│    │
     *   │    │
     *   └────┘
     */

    // link em' up
    iter(x, WORLD_SIZE) { 
        iter(y, WORLD_SIZE) {
            if (y+1 < WORLD_SIZE)
                world[x][y]->down = world[x][y+1];
            else
                world[x][y]->down = hole;

            if (y-1 >= 0)
                world[x][y]->up = world[x][y-1];
            else
                world[x][y]->up = hole;

            if (x+1 < WORLD_SIZE)
                world[x][y]->right = world[x+1][y];
            else
                world[x][y]->right = hole;

            if (x-1 >= 0)
                world[x][y]->left = world[x-1][y];
            else
                world[x][y]->left = hole;
        }
    }

    double num_sites = WORLD_SIZE*WORLD_SIZE;
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
                scan_for_strings(world);
                double seconds_passed = seconds_to_render + seconds_to_update + seconds_to_wait;
                printf("AER = %6.6f , FPS = %6.6f\n", ((double)sps)/(seconds_passed * num_sites),
                        1.0/seconds_passed);
                printf("render = %6.6f ms, physics = %6.6f ms, wait = %6.6f ms\n", seconds_to_render*1000, 
                        seconds_to_update*1000, seconds_to_wait*1000);
            }

        }

        // X keyboard and mouse input
        if (poll_events(&sps, world))
            return 1;
    }
}


