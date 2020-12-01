#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "constants.h"

extern cell_t* hole;

// these tags are replaced by prepro2.py
#ELEMENT_ENUM
#ELEMENT_COLORS
#ELEMENT_NAMES

// HELPER FUNC AREA

cell_t* random_neighbor(cell_t* cell) {
    if (cell == hole) {
        return hole;
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
    return hole;
}

cell_t* random_neighbor8(cell_t* cell) {
    if (cell == hole) {
        return hole;
    }
    switch (rand() % 8) {
        case 0:
            return cell->right;
        case 1:
            return cell->left;
        case 2:
            return cell->up;
        case 3:
            return cell->down;
        case 4:
            return cell->up->right;
        case 5:
            return cell->up->left;
        case 6:
            return cell->down->right;
        case 7:
            return cell->down->left;
    }
    return hole;
}



int is_empty(cell_t* cell) {
    return cell != hole && cell->type == BLANK;
}
int is_taken(cell_t* cell) {
    return cell != hole && cell->type != BLANK;
}

void nice_copy_cell(cell_t* dest, cell_t* source) {
    if (source != hole && (is_empty(dest))) {
        dest->type = source->type;
        memcpy(dest->data, source->data, DATA_SIZE);
    }
}

// assumes both are non-hole
void copy_cell(cell_t* dest, cell_t* source) {
    dest->type = source->type;
    memcpy(dest->data, source->data, DATA_SIZE);
}


void swap_cells(cell_t* a, cell_t* b) {
    if (a == hole)
        return;
    if (b == hole)
        return;
    cell_t* temp = malloc(sizeof(cell_t));
    copy_cell(temp, a);
    copy_cell(a, b);
    copy_cell(b, temp);
    free(temp);
}

void move_to_target_if_empty(cell_t* target, cell_t* source) {
    if (is_empty(target)) {
        memcpy(target->data, source->data, DATA_SIZE);
        target->type = source->type;
        memset(source->data, 0, DATA_SIZE);
        source->type = BLANK;
    }
}


void clear_cell(cell_t* cell) {
    memset(cell->data, 0, DATA_SIZE);
    cell->type = BLANK;
}

void clear_and_set_type_if_cell_is_empty(cell_t* cell, char type) {
    if (is_empty(cell)) {
        clear_cell(cell);
        cell->type = type;
    }
}

void diffuse(cell_t * cell, int probability) {
    if (rand() % probability == 0) {
        move_to_target_if_empty(random_neighbor8(cell), cell);
    }
}

void hard_diffuse(cell_t * cell, int probability) {
    if (rand() % probability == 0) {
        swap_cells(random_neighbor8(cell), cell);
    }
}

int randp (int n) {
    return rand() % n == 0;
}





// start inclusive, end exclusive
int randrange(int start, int end) {
    return (rand() % (end - start)) + start;
}


void dont_update(cell_t* cell) {
    (void)(cell);
}

unsigned char dec(unsigned char x) {
    if (x > 0)
        return x-1;
    return 0;
}

unsigned char inc(unsigned char x) {
    if (x < 255)
        return x + 1;
    return x;
}


// this tag is replaced by prepro2.py
#UPDATE_FUNCTIONS

void step(world_t world, int x, int y) {
    cell_t* cell = world[x][y];


    update_function_type update_function = update_functions[cell->type];
    if (update_function != NULL) {
        (*update_function)(cell);
    } else {
        clear_cell(cell);
    }
}
