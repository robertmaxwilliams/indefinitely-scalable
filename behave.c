#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "constants.h"

extern cell_t* hole;
//                 0      1          2         3    4    5     6    7      8   9
enum CELL_TYPES {BLANK=0, STRING=1, ISOLATER, DREG, RES, WALL, REM, MAKER, BOX, STRING_COPIER, 
    DOWN_DROPPER, SHORT_LIVED, FALLER, STRINGS_SPLITTER, RISER}; 

unsigned int cell_colors[256] = {
    0xA9CCE3, // blank/background 0 sky blue
    0x5D6D7E, // 1 string steel
    0x95A5A6, // 2  string isolater grey
    0xD35400, // 3 dreg orange
    0x58D68D, // 4 res lime
    0x1B2631, // 5 wall black
    0x641E16, // 6 rememberer burnt red
    0xD4AC0D, // 7 maker, dark yellow
    0x145A32, // 8 box, dark green
    0x9B59B6, // 9 string copier, purple
    0x154360, // 10 down dropper, dark navy
    0xE5E7E9, // 11 short lived, almost white
    0x4A235A, // 12 faller, dark purple
    0xE74C3C, // 13 strings splitter, bright red
    0x5DADE2, // 14 riser, blue
};

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
        case BLANK:
            if (randp(10)) 
                // sometimes make fresh res
                a->type = RES;
            else
                // diffuse in open space
                swap_cells(a, cell);
            break;

        case RES:
            if (randp(500)) 
                // rarely convert res to dreg
                a->type = DREG;
            else
                // destroy res
                clear_cell(a);
            break;

        case DREG:
            // sometimes convert dreg to res
            if (randp(10)) a->type = RES;
            break;

        default:
            if (randp(2)) {
                // convert anyone to res
                clear_cell(a);
                a->type = RES;
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

// data is {id, string index, character}
// change a string to this type to make it fall.
void update_faller(cell_t* cell) {
    PATTERN
        |  .
        | aba;
    if (randp(6)) {
        if (randp(5)) {
            move_to_target_if_empty(b, cell);
        } else {
            move_to_target_if_empty(a, cell);
        }
    }
}

void update_riser(cell_t* cell) {
    PATTERN
        | aba
        |  .;
    if (randp(6)) {
        if (randp(5)) {
            move_to_target_if_empty(b, cell);
        } else {
            move_to_target_if_empty(a, cell);
        }
    }
}


// helper for update_rememberer
void remember_and_place(cell_t* cell, unsigned char i, cell_t* neigh, cell_t* maybe_res) {
    if (neigh->type == REM) {
        cell->data[i%10] = 1;
    } else if (maybe_res->type == RES && cell->data[i%10] == 1) {
        swap_cells(neigh, maybe_res);
        clear_cell(neigh);
        neigh->type = REM;
    }
}

// stores whether there is someone at various relative positions, 
// in the following way: {top, topright, right, rightbuttom, bottom, bl, left, lt, empty, rot}
void update_rememberer(cell_t* cell) {
    cell->data[9] = rand() % 4;
    unsigned char rot = cell->data[9];

    PATTERN ROTATE rot
        |  r
        | rrr
        |  ab
        |  .
        | rrr
        |  r;

    cell_t* maybe_res = hole;
    if (r->type == RES) maybe_res = r;
    // rot*2 + (1 or 0) will be from 0 to below 8
    remember_and_place(cell, rot*2, a, maybe_res);
    remember_and_place(cell, rot*2+1, b, maybe_res);
}

// stores: {n steps, rot}
void update_box(cell_t* cell) {
    unsigned char rot = cell->data[1];
    unsigned char steps = cell->data[0];

    PATTERN ROTATE rot
        |  rrr
        |  b.a
        |  rrr;
    if (steps >= 10) {
        // programmed death if in an unspecified state
        clear_cell(cell);
        cell->data[0] = 126; // mark for debugging
        cell->type = RES;
        return;
    }

    if (r->type == RES) {
        if (is_empty(a)) {
            r->type = BLANK;
            a->type = BOX;
            if (steps == 9) {
                a->data[0] = 0;
                a->data[1] = (rot + 1) % 4;
            } else {
                a->data[0] = steps + 1;
                a->data[1] = rot;
            }
        } else if (is_empty(b) && steps != 0) { // TODO linear leaves weak spot
            r->type = BLANK;
            b->type = BOX;
            b->data[0] = steps - 1;
            b->data[1] = rot;
        }
    }
}



// start inclusive, end exclusive
int randrange(int start, int end) {
    return (rand() % (end - start)) + start;
}


// data is {tracker, stored index, stored character, new string tracker}
//           0         1               2               3             
void update_string_copier(cell_t* cell) {
    PATTERN
        |   a
        |  aaa
        |   .  
        |  xxx
        |   x;
    // on creation, imprint self with a random id
    if (cell->data[3] == 0) {
        cell->data[3] = randrange(1, 256);
    }

    if (a->type == STRING) {
        // copy a string into self but use our ID
        memcpy(cell->data, a->data, 3);
        cell->data[0] = cell->data[3];
    } else if (a->type == RES && cell->data[0] != 0 && is_empty(x)) {
        // use res to build the string if we have one inside then delete our copy
        clear_cell(a);
        memcpy(x->data, cell->data, 3);
        x->type = STRING;
        cell->data[0] = 0;
    }
}

// data is {confidence}
void update_downdropper(cell_t* cell) {

    if (randp(2)) {
        // determine whether its safe to move down
        PATTERN
             aaaaa
               .
               x;

        if (a->type != DOWN_DROPPER)
            cell->data[0] += 1;
        else
            cell->data[0] = 0;

        if (cell->data[0] > 15 && is_empty(x)) {
            cell->data[0] = 0;
            swap_cells(cell, x);
        }

    } else {
        // move stuff down
        PATTERN
            |   aaa 
            |  aa.aa
            |   aaa
            |    x ;
        if (is_empty(x)) {
            if (a->type == RES || a->type == STRING) {
                // move res or string downward somewhat
                swap_cells(a, x);
            }
        }
    }
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
    if (a->type == RES && cell->data[0] != 0) {
        copy_cell(a, cell);
        a->type = STRING;
        return;
    }
    if (a->type != STRING)
        return;
    unsigned char tracker = cell->data[0];

    if (tracker == 0) {
        // if we don't have a tracker, track the string we first encounter
        cell->data[0] = a->data[0];
    } else {
        // turn an enemy string into a res
        if (a->data[0] != tracker) {
            clear_cell(a);
            a->type = RES;
        // copy friendly data into store
        } else {
            cell->data[1] = a->data[1];
            cell->data[2] = a->data[2];
        }
    }
}


// makes blank copies of whateber type it sees first
// data is {target}
void update_something_maker(cell_t* cell) {
    if (cell->data[0] == 0) {
        PATTERN
            | a
            |a.a
            | a;

        if (a->type != RES && is_taken(a)) {
            cell->data[0] = a->type;
        }
    } else {
        PATTERN
            | r
            |r.r
            | x;

        if (r->type == RES && is_empty(x)) {
            x->type = cell->data[0];
            r->type = BLANK;
        }
    }
}

void update_short_lived(cell_t* cell) {
    if (randp(2)) {
        cell->data[0] += 1;
        if (cell->data[0] > 100) {
            clear_cell(cell);
            cell->type = RES;
        }
    } else {
        diffuse(cell,8);
    }
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

// data is {imprint1, imprint2, deather}
// imprint2 is made into down_dropper
// imprint1 is made into up_riser or whatever
// deather is incremented when we don't see a string and reset when we do
void update_strings_splitter(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    if (cell->data[2] > 100) {
        cell->type = RES;
        return;
    }
    if (a->type == STRING) {
        unsigned char string_id = a->data[0];
        if (cell->data[0] == 0) {
            cell->data[0] = string_id;
        } else if (cell->data[1] == 0) {
            if (string_id != cell->data[0])
                cell->data[1] = string_id;
        } else {
            cell->data[2] = 0; // reset death clock
            if (string_id == cell->data[1])
                a->type = FALLER;
            else if (string_id == cell->data[0])
                a->type = RISER;
        }
    } else if (a->type == RES) {
        if (randp(5)) cell->data[2] = inc(cell->data[2]);
        if (cell->data[0] != 0 && cell->data[1] != 0)
            copy_cell(a, cell);
    } else {
        if (randp(5)) cell->data[2] = inc(cell->data[2]);
    }
    diffuse(cell, 3);
}

/*
// data is {imprint, split location}
#define imprint cell->data[0]
#define split_location cell->data[1]
void update_split_on_space(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    if (a->type == STRING) {
        if (imprint == 0) {
            imprint = a->data[0];
        } else {
            if (a->data[2] == ' ') {
                split_location == a->

*/


update_function_type update_functions[256] = {
    dont_update,
    update_string,
    update_isolater,
    update_dreg,
    update_res,
    dont_update,
    update_rememberer,
    update_something_maker,
    update_box,
    update_string_copier,
    update_downdropper,
    update_short_lived,
    update_faller,
    update_strings_splitter,
    update_riser,
    // the rest should be void
};

void step(world_t world, int x, int y) {
    cell_t* cell = world[x][y];


    update_function_type update_function = update_functions[cell->type];
    if (update_function != NULL) {
        (*update_function)(cell);
    } else {
        clear_cell(cell);
    }
}
