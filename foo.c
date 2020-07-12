#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "constants.h"

#include "graphics.h"

#define iter(i_var, i_limit) for (int i_var=0;i_var<i_limit;i_var++)

unsigned int color_table[256];
int sps = 500000;
int placer_index = 1;
char placer_types[] = " dt1x           ";

typedef struct _cell_t {
    struct _cell_t* up;
    struct _cell_t* down;
    struct _cell_t* left;
    struct _cell_t* right;
    char type;
} cell_t;
typedef cell_t *grid_t[GRID_SIZE][GRID_SIZE];
typedef grid_t *world_t[WORLD_SIZE][WORLD_SIZE];

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
                    draw(n*GRID_SIZE+j, m*GRID_SIZE+i, color_table[(*grid)[i][j]->type]);
                }
            }
        }
    }
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
}


cell_t* random_neighbor(cell_t* cell) {
    if (cell == NULL) {
        return NULL;
    }
    switch (rand() % 4) {
        case 0:
            return cell->right;
        case 1:
            return cell->left;
        case 2:
            return cell->up;
        case 3:
            return cell->down;
    }
    return NULL;
}

int is_empty(cell_t* cell) {
    return cell != NULL && cell->type == ' ';
}
int is_taken(cell_t* cell) {
    return cell != NULL && cell->type != ' ';
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
            printf("%d %d\n", x, y);
            int i = y % GRID_SIZE;
            int j = x % GRID_SIZE;
            int m = y / GRID_SIZE;
            int n = x / GRID_SIZE;
            (*(*world)[m][n])[i][j]->type = placer_types[placer_index];
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
            }
        }
    }
    return 0;
}

#define break_if_nonempty(cell) if (!is_empty(cell)) break
void step(grid_t* grid) {
    int i = rand() % GRID_SIZE;
    int j = rand() % GRID_SIZE;
    cell_t* cell = (*grid)[i][j];
    cell_t* neigh; //todo move updates into functons, maybe in seperate file
    switch (cell->type) {
        case 'x':
            neigh = random_neighbor(cell);
            break_if_nonempty(neigh);
            neigh->type = 'x';
            break;
        case 'd':
            neigh = random_neighbor(cell);
            if (is_empty(neigh)) {
                neigh->type = 'd';
                cell->type = 't';
            } /*else if (is_taken(neigh)) {
                cell->type = neigh->type;
                neigh->type = 'd';
            }*/
            break;
        case 't':
            neigh = random_neighbor(cell);
            break_if_nonempty(neigh);
            neigh->type = 't';
            cell->type = ' ';
            break;
        case '1':
            if (rand() % 10000 == 0) {
                cell->type = 't';
                break;
            }
            neigh = random_neighbor(cell);
            if (is_taken(neigh) && neigh->type == 't') {
                neigh->type = '1';
            }
            break;
    }
}

int main() {
    //bbggrraa
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
                    (*gridp)[i][j]->type = ' ';
                }
            }
        }
    }
    (*world[1][1])[1][1]->type = '1';
    (*world[3][3])[1][1]->type = 'd';
    (*world[14][3])[1][1]->type = '1';

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
                        (*world[m][n])[i][j]->down = NULL;
                    }

                    // i up (negative)
                    if (i-1 >= 0) {
                        (*world[m][n])[i][j]->up = 
                            (*world[m][n])[i-1][j];
                    } else if (m-1 >= 0) {
                        (*world[m][n])[i][j]->up = 
                            (*world[m-1][n])[GRID_SIZE-1][j];
                    } else {
                        (*world[m][n])[i][j]->up = NULL;
                    }

                    // j right (positive)
                    if (j+1 < GRID_SIZE) {
                        (*world[m][n])[i][j]->right = 
                            (*world[m][n])[i][j+1];
                    } else if (n+1 < WORLD_SIZE) {
                        (*world[m][n])[i][j]->right = 
                            (*world[m][n+1])[i][0];
                    } else {
                        (*world[m][n])[i][j]->right = NULL;
                    }

                    // j left (negative)
                    if (j-1 >= 0) {
                        (*world[m][n])[i][j]->left = 
                            (*world[m][n])[i][j-1];
                    } else if (n-1 >= 0) {
                        (*world[m][n])[i][j]->left = 
                            (*world[m][n-1])[i][GRID_SIZE-1];
                    } else {
                        (*world[m][n])[i][j]->left = NULL;
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

    while (1) {

        if (foo%sps == 0) {
            clock_t now_time = clock();
            clock_t time_diff = now_time - last_time;

            float seconds_passed = ((float) time_diff)/ CLOCKS_PER_SEC;
            printf("AER = %6.6f , FPS = %6.6f\n", ((float)num_events)/(seconds_passed * num_sites),
                    1.0/seconds_passed);
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


