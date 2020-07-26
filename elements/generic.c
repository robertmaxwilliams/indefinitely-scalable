/*
 * Called generic because they will function independently of whatever it is 
 * they're operating on.
 */

// makes blank copies of whateber type it sees first
// data is {target}
#ELEMENT MAKER 0xD4AC0D, // maker, dark yellow
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

#ELEMENT SHORT_LIVED 0xE5E7E9 // short lived, almost white
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


// data is {id, string index, character}
// change a string to this type to make it fall.
#ELEMENT FALLER 0x4A235A // faller, dark purple
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

#ELEMENT RISER 0x5DADE2 // riser, blue
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

// data is {confidence}
#ELEMENT DOWN_DROPPER 0x154360 // down dropper, dark navy
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
#ELEMENT ISOLATOR 0x95A5A6 // isolater grey
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



