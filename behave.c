#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"

extern cell_t* hole;

// HELPER FUNC AREA, TODO put in external file

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
    return cell != hole && cell->type == ' ';
}
int is_taken(cell_t* cell) {
    return cell != hole && cell->type != ' ';
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
}

void move_to_target_if_empty(cell_t* target, cell_t* source) {
    if (is_empty(target)) {
        memcpy(target->data, source->data, DATA_SIZE);
        target->type = source->type;
        memset(source->data, 0, DATA_SIZE);
        source->type = ' ';
    }
}

void clear_cell(cell_t* cell) {
    memset(cell->data, 0, DATA_SIZE);
    cell->type = ' ';
}


void diffuse(cell_t * cell, int probability) {
    if (rand() % probability == 0) {
        move_to_target_if_empty(random_neighbor8(cell), cell);
    }
}

int randp (int n) {
    return rand() % n == 0;
}

/* 
 * CELL FUNC AREA
 *
 * cells:
 * isolater i
 * string s
 * dreg d
 * res r
 */

    
void update_dreg(cell_t* cell) {
    if (randp(500)) {
        diffuse(cell, 1);
        return;
    }
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa
        |   a;
    switch (a->type) {
        case ' ':
            // make fresh res
            if (randp(50000)) a->type = 'r';
            break;
        case 'r':
            // convert res to dreg
            if (randp(50000)) a->type = 'd';
            // destroy res
            if (randp(500)) clear_cell(a);
            break;
        case 'd':
            // convert dreg to res
            if (randp(50)) a->type = 'r';
            break;
        default:
            // convert anyone to res
            if (randp(500)) {
                clear_cell(a);
                a->type = 'r';
            }
    }
}

void update_res(cell_t* cell) {
    diffuse(cell, 100);
}

// data is {id, string index, character}
void update_string(cell_t* cell) {
    diffuse(cell, 1000);
}

// data is {tracker, stored index, stored character}
void update_isolater(cell_t* cell) {
    if (randp(500)) {
        diffuse(cell, 1);
        return;
    }
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa
        |   a;
    // consume a res to replicate self or stored string
    if (a->type == 'r') {
        if (randp(2)) {
            copy_cell(a, cell);
        } else {
            copy_cell(a, cell);
            a->type = 's';
        }

        return;
    }
    if (a->type != 's')
        return;
    unsigned char tracker = cell->data[0];

    if (tracker == 0) {
        // if we don't have a tracker, track the string we first encounter
        cell->data[0] = a->data[0];
    } else {
        // turn an enemy string into a res
        if (a->data[0] != tracker) {
            clear_cell(a);
            a->type = 'r';
        // copy friendly data into store
        } else {
            cell->data[1] = a->data[1];
            cell->data[2] = a->data[2];
        }
    }
}




void step(grid_t* grid) {
    int i = rand() % GRID_SIZE;
    int j = rand() % GRID_SIZE;
    cell_t* cell = (*grid)[i][j];
    switch (cell->type) {
        case 'i': update_isolater(cell); break;
        case 'd': update_dreg(cell); break;
        case 'r': update_res(cell); break;
        case 's': update_string(cell); break;
        default:
            clear_cell(cell);
    }
}
