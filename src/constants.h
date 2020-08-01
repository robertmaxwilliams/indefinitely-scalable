#ifndef CONSTANTS_HEADER_H 
#define CONSTANTS_HEADER_H

// erebody needs these bad boys
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define WORLD_SIZE 30
#define SCALE 16
#define BORDER  (SCALE/4)
#define SIDEBAR  30
#define NUM_SIDEBAR_COLUMNS 5
#define WINDOW_HEIGHT (WORLD_SIZE * SCALE)
#define NUM_SIDEBAR_ROWS (WINDOW_HEIGHT / SIDEBAR)
#define WINDOW_WIDTH ((WORLD_SIZE * SCALE) + NUM_SIDEBAR_COLUMNS * SIDEBAR)

#define iter(i_var, i_limit) for (int i_var=0;i_var<i_limit;i_var++)
#define ifrand(n) if (rand() % n == 0)
#define DATA_SIZE 10



typedef struct _cell_t {
    unsigned char type;
    unsigned char data[DATA_SIZE]; // data is only copied around when the cell wants to
    struct _cell_t* up;
    struct _cell_t* down;
    struct _cell_t* left;
    struct _cell_t* right;
} cell_t;

typedef cell_t *world_t[WORLD_SIZE][WORLD_SIZE];

// hole is used like NULL except it points to itself, so hole->left is hole, and so on.
cell_t* hole;

typedef void (*update_function_type)(cell_t*);


#endif

