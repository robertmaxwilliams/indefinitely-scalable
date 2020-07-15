#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"

extern cell_t* hole;

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
    

void update_string(cell_t* cell) {
    diffuse(cell, 1000);
}

// sorts left to right
// keeps data in data[0]
void update_sorter(cell_t* cell) {
    ifrand(1000) {
        diffuse(cell, 1);
    }
    ifrand(6) {
#PATTERN
            aaa  
            a.a
            aaa  
#ENDPATTERN

            // diffuse when near other sorters
            if (a->type == 's') {
                diffuse(cell, 1);
                ifrand(100) {
                    clear_cell(cell);
                }
            } else if (a->type == 'd') {
                clear_cell(cell);
            }
            return;
    }
#PATTERN
   xx aa  
   xx aa  
   zz.
   yy aa  
   yy aa  
#ENDPATTERN

   // make sure we're sorting data
   if (a->type != 't')
       return;

   // do one unit of sorting
   cell_t* dest;
   if (a->data[0] > cell->data[0])
       dest = x;
   else if (a->data[0] < cell->data[0])
       dest = y;
   else
       dest = z;
   // only sort if dest is empty
   if (!is_empty(dest))
       return;
   cell->data[0] = a->data[0];
   swap_cells(dest, a);

}





//       0      1      2    3
// goes right, down, left, right. [0] is distance and [1] is state
void update_box(cell_t* cell) {
#PATTERN
    c
   a.b
    d
#ENDPATTERN

    if (cell->data[0] > 20) {
        cell->data[1] = (cell->data[1] + 1) % 4;
        cell->data[0] = 0;
    }

    cell_t* dest;
    switch (cell->data[1]) {
        case 0: dest = b; break;
        case 1: dest = d; break;
        case 2: dest = a; break;
        case 3: dest = c; break;
        default: return;
    }
    if (is_empty(dest)) {
        copy_cell(dest, cell);
        dest->data[0] += 1;
    }
}

void update_line(cell_t* cell) {
#PATTERN
    s.s
#ENDPATTERN
    if (is_empty(s)) {
        copy_cell(s, cell);
    }
}

void update_datum(cell_t* cell) {
    if (rand() % 100 > 0) return;
#PATTERN
   a 
   a.
   a 
#ENDPATTERN

    if (is_empty(a)) {
        swap_cells(a, cell);
    }
}    

void update_consumer(cell_t* cell) {
#PATTERN
    aa
    .a
    aa
#ENDPATTERN
    if (a->type == 't') {
        clear_cell(a);
    }
}



#define break_if_nonempty(cell) if (!is_empty(cell)) break
void step(grid_t* grid) {
    int i = rand() % GRID_SIZE;
    int j = rand() % GRID_SIZE;
    cell_t* cell = (*grid)[i][j];
    cell_t* neigh; //todo move updates into functons, maybe in seperate file
    switch (cell->type) {
        case 's': update_sorter(cell); break;
        case 'y': update_box(cell); break;
        case 'f': update_line(cell); break;
        case 't': update_datum(cell); break;
        case 'c': update_consumer(cell); break;
        case 'i': update_string(cell); break;
        case 'x':
            if (cell->data[0] > 20)
                break;
            neigh = random_neighbor(cell);
            break_if_nonempty(neigh);
            copy_cell(neigh, cell);
            neigh->data[0] += 1;
            break;
        case 'd': // data dispenser
            neigh = random_neighbor(cell);
            if (is_empty(neigh)) {
                neigh->type = 't';
                neigh->data[0] = rand() % 256;
            }
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
        default:
            cell->type = ' ';
            memset(cell->data, 0, DATA_SIZE);
    }
}
