/*
 * String manipulation operators, I'd like to have more
 * of these.
 */
// data is {tracker, stored index, stored character, new string tracker}
//           0         1               2               3             
#ELEMENT STRING_COPIER 0x9B59B6 // string copier, purple
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

// data is {imprint1, imprint2, deather}
// imprint2 is made into down_dropper
// imprint1 is made into up_riser or whatever
// deather is incremented when we don't see a string and reset when we do
#ELEMENT STRING_SPLITTER 0xE74C3C // strings splitter, bright red
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

