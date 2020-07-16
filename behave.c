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

void hard_diffuse(cell_t * cell, int probability) {
    if (rand() % probability == 0) {
        swap_cells(random_neighbor8(cell), cell);
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
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa
        |   a;

    switch (a->type) {
        case ' ':
            if (randp(10)) 
                // sometimes make fresh res
                a->type = 'r';
            else
                // diffuse in open space
                swap_cells(a, cell);
            break;

        case 'r':
            if (randp(500)) 
                // rarely convert res to dreg
                a->type = 'd';
            else
                // destroy res
                clear_cell(a);
            break;

        case 'd':
            // sometimes convert dreg to res
            if (randp(10)) a->type = 'r';
            break;

        default:
            if (randp(2)) {
                // convert anyone to res
                clear_cell(a);
                a->type = 'r';
            } 
            /*
            else {
                // or swap with them 
                swap_cells(a, cell);
            }
            */
    }
}

void update_res(cell_t* cell) {
    diffuse(cell, 3);
}

// data is {id, string index, character}
void update_string(cell_t* cell) {
    diffuse(cell, 6);
}


// helper for update_rememberer
void remember_and_place(cell_t* cell, int i, cell_t* neigh, cell_t* maybe_res) {
    if (neigh->type == 'm') {
        cell->data[i] = 1;
    } else if (maybe_res->type == 'r' && cell->data[i] == 1) {
        swap_cells(neigh, maybe_res);
        clear_cell(neigh);
        neigh->type = 'm';
    }
}

// stores whether there is someone at various relative positions, 
// in the following way: {a, b, c, d}
void update_rememberer(cell_t* cell) {
    PATTERN
        |      w 
        |     www
        |    zeafx
        |   zzd.bxx   
        |    zhcgx
        |     yyy
        |      y;
    cell_t* maybe_res = hole;
    if (w->type == 'r') maybe_res = w;
    else if (x->type == 'r') maybe_res = x;
    else if (y->type == 'r') maybe_res = y;
    else if (z->type == 'r') maybe_res = z;
    remember_and_place(cell, 0, a, maybe_res);
    remember_and_place(cell, 1, b, maybe_res);
    remember_and_place(cell, 2, c, maybe_res);
    remember_and_place(cell, 3, d, maybe_res);
    remember_and_place(cell, 4, e, maybe_res);
    remember_and_place(cell, 5, f, maybe_res);
    remember_and_place(cell, 6, g, maybe_res);
    remember_and_place(cell, 7, h, maybe_res);
}


// data is {tracker, stored index, stored character}
void update_isolater(cell_t* cell) {
    if (randp(6)) {
        diffuse(cell, 1);
        return;
    }
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa
        |   a;
    // consume a res to stored string if present
    if (a->type == 'r' && cell->data[0] != 0) {
        copy_cell(a, cell);
        a->type = 's';
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
        case 'm': update_rememberer(cell); break;
        case 'i': update_isolater(cell); break;
        case 'd': update_dreg(cell); break;
        case 'r': update_res(cell); break;
        case 's': update_string(cell); break;
        case 'w': break;
        default:
            clear_cell(cell);
    }
}
