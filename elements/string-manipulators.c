/*
 * String manipulation operators, I'd like to have more
 * of these.
 */
typedef struct { 
    char type; 
    char id; 
    char index; 
    char character; 
    char length;
} string_t;

typedef struct { 
    char type; 
    char id; 
    char index; 
    char character; 
    char length;
} frozen_string_t;


typedef struct {
    char type;
    char orientation;
    char id;
    char is_working;
    char index;
} cell_to_string_t;

#ELEMENT CELL_TO_STRING
void update_cell_to_string(cell_t* cell) {
    cell_to_string_t* self = (void*) cell;
    PATTERN ROTATE self->orientation
        |   r
        |  rrr
        | rr.ar
        |  rrr
        |   r;
    if (r->type == RES) {
        if (!self->is_working && a->type != BLANK && a->type != RES && a->type != DREG
                && a->type != STRING && a->type != WALL) {
            self->is_working = 1;
            self->id = randrange(1, 256);
            r->type = STRING;
            string_t* string = (void*) r;
            string->length = 11;
            string->id = self->id;

            string->character = a->type;
            a->type = WALL;

            string->index = 0;
            self->index = 1;
        } else if (self->is_working) {
            r->type = STRING;
            string_t* string = (void*) r;
            string->length = 11;
            string->id = self->id;
            string->character = a->data[(self->index - 1)%10];
            string->index = self->index;
            self->index += 1;
            if (self->index >= 11) {
                clear_cell(cell);
                cell->type = RES;
                return;
            }
        }
    }

    if (!self->is_working) {
        self->orientation = randrange(0, 4);
        diffuse(cell, 3);
    }
}




typedef struct {
    char type;
    char orientation;
    char id;
    char is_working;
    char index;
    char stored_type;
    short unsigned int mask;
} string_to_cell_t;

#ELEMENT string_to_cell
void update_string_to_cell(cell_t* cell) {
    string_to_cell_t* self = (void*) cell;
    PATTERN ROTATE self->orientation
        |   s
        |  sss
        | ss.rs
        |  sss
        |   s;
    string_t* string = (void*) s;
    if (r->type == RES && !self->is_working && s->type == STRING && string->length == 11) {
            self->id = string->id;
            self->is_working = 1;
            self->stored_type = r->type;
            r->type = WALL;
    } else if (self->is_working && s->type == STRING && string->id == self->id) {
        char i = string->index;
        r->data[i%10] = string->character;
        self->mask |= 1 << i;
        if (self->mask == 0x7ff) { // 0x7ff == 0b_0000_0111_1111_1111
            r->type = self->stored_type;
            clear_cell(cell);
            cell->type = RES;
        } else if (self->mask | 0xF800) {
            clear_cell(cell);
            cell->type = RES;
        }
    } else if (!self->is_working) {
        self->orientation = randrange(0, 4);
        diffuse(cell, 3);
    }
}






typedef struct {
    char type;
    char imprint;
    char cur_index;
} string_freezer_t;

#ELEMENT STRING_FREEZER
void update_string_freezer(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    string_freezer_t* self = (void*) cell;

    if (a->type == STRING) {
        string_t* string = (void*) a;
        if (self->imprint == 0) {
            self->imprint = string->id;
        } else if (self->imprint == string->id && string->index == self->cur_index) {
            string->type = FROZEN_STRING;
            self->cur_index += 1;
            if (self->cur_index >= string->length) {
                clear_cell(cell);
                self->type = RES;
            }
            swap_cells(cell, a);
        }
    }
}





typedef struct {
    char type;
    char source_id;
    char stored_index;
    char stored_character;
    char stored_length;
    char target_id;
    char count;
    char has_one_stored;
} string_copier_t;
#ELEMENT STRING_COPIER 0x9B59B6 // string copier, purple
void update_string_copier(cell_t* cell) {
    PATTERN
        |  a
        | a.a  
        |  a;

    string_copier_t* self = (void*) cell;

    if (a->type == STRING) {
        string_t* string = (void*) a;
        if (self->source_id == 0) {
            self->source_id = string->id;
            self->target_id = randrange(1, 256);
        } else if (string->id == self->source_id && string->index == self->count) {
            // copy a string into self but use our ID
            self->stored_index = string->index;
            self->stored_character = string->character;
            self->stored_length = string->length;
            self->has_one_stored = 1;
        }
    } else if (a->type == RES && self->has_one_stored) {
        // use res to build the string if we have one inside then delete our copy
        clear_cell(a);
        a->type = STRING;
        string_t* string = (void*) a;
        string->id = self->target_id;
        string->index = self->stored_index;
        string->character = self->stored_character;
        string->length = self->stored_length;
        self->count += 1;
        self->has_one_stored = 0;
        if (self->count >= self->stored_length) {
            clear_cell(cell);
            cell->type = RES;
        }
    }
    diffuse(cell, 1);
}

typedef struct { 
    char type;
    char imprint1;
    char imprint2;
    char cur_index;
    char found1;
    char character1;
} string_equal_t;

#ELEMENT STRING_EQUAL
void update_string_equal(cell_t* cell) {
    PATTERN
        |   a
        |  a.a
        |   a;

    // void case cell to "self" with named fields
    string_equal_t* self = (void*) cell;

    if (a->type == STRING) {
        string_t* string = (void*) a;
        if (self->imprint1 == 0) {
            self->imprint1 = string->id;
        } else if (self->imprint2 == 0) {
            if (string->id != self->imprint1) {
                self->imprint2 = string->id;
            }
        } else {
            if (!self->found1 && string->id == self->imprint1 && string->index == self->cur_index) {
                self->character1 = string->character;
                self->found1 = 1;
            } else if (self->found1 && string->id == self->imprint2 && string->index == self->cur_index) {
                if (self->character1 == string->character) {
                    self->cur_index += 1;
                    self->found1 = 0;
                    if (self->cur_index == string->length) {
                        cell->type = STRING;
                        cell->data[0] = randrange(1, 256);
                        cell->data[1] = 0;
                        cell->data[2] = 'y';
                        cell->data[3] = 1;
                    }
                } else {
                    printf("failed at %d, %c(%d) != %c(%d)\n", self->cur_index, self->character1, 
                            self->character1, string->character, string->character);
                    cell->type = STRING;
                    cell->data[0] = randrange(1, 256);
                    cell->data[1] = 0;
                    cell->data[2] = 'n';
                    cell->data[3] = 1;
                }
            }
        }
    }
    diffuse(cell, 2);
}




/* 
 * Steps are:
 *      - while you don't have an imprint, look for a string
 *      - imprint on the string, start looking for the element with the largest pos
 *      - once you've not seen a larger pos for a while (boredom), start
 *          finding res and copying the string into reversed one
 */
// data is {imprint, destination id, max pos, current pos, str_char}
typedef struct {
        char type;
        char imprint;
        char destination_id;
} string_reverser_t;

#ELEMENT STRING_REVERSER 0xff0000
void update_string_reverser(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    // unsafe cast our cell and the string to a new type with better names
    string_reverser_t* self = (void*) cell;


    if (a->type == STRING) { 
        string_t * string = (void*) a;
        if (self->imprint == 0) {
            self->imprint = string->id;
            self->destination_id = randrange(1, 256);
        } else if (string->id == self->imprint) {
            string->id = self->destination_id;
            string->index = string->length - string->index - 1;
            }
    }
    diffuse(cell, 1);
}

typedef struct {
        char type;
        char imprint;
        char destination_id;
        char cur_index;
} frozen_string_reverser_t;

#ELEMENT FROZEN_STRING_REVERSER
void update_frozen_string_reverser(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    // unsafe cast our cell and the string to a new type with better names
    frozen_string_reverser_t* self = (void*) cell;


    if (a->type == FROZEN_STRING) { 
        string_t * string = (void*) a;
        if (self->imprint == 0 && string->index == 0) {
            self->imprint = string->id;
            self->destination_id = randrange(1, 256);
        } else if (string->id == self->imprint && string->index == self->cur_index) {
            string->id = self->destination_id;
            string->index = string->length - string->index - 1;
            self->cur_index += 1;
            if (self->cur_index >= string->length) {
                clear_cell(cell);
                cell->type = RES;
            }
            swap_cells(a, cell);
        }
    }
    if (self->imprint == 0) {
        diffuse(cell, 1);
    }
}

typedef struct {
        char type;
        char imprint;
        char destination_id;
        char cur_index;
        char split_start_index;
        char is_backward;
        char stored_index;
        char stored_length;
} frozen_string_to_words_t;

// splits strings on spaces, each space resets index counter and randomizes ID
// Has to make two passes, forward splitting and writing lengths to ' ' chars,
// the backwards propogating the lengths
#ELEMENT FROZEN_STRING_TO_WORDS
void update_frozen_string_to_words(cell_t* cell) {
    PATTERN
        |   a
        |  a.a
        |   a;
    frozen_string_to_words_t* self = (void*) cell;

    if (a->type == FROZEN_STRING) {
        string_t * string = (void*) a;
        if (self->imprint == 0 && string->index == 0) {
            self->imprint = string->id;
            self->destination_id = randrange(1, 256);
        } else if (!self->is_backward && string->id == self->imprint && string->index == self->cur_index) {
            char length = string->length;
            if (string->character == ' ') {
                string->length = self->cur_index - self->split_start_index;
                self->split_start_index = string->index + 1;
            }
            self->cur_index += 1;

            if (self->cur_index >= length) {
                self->stored_index = self->cur_index - self->split_start_index - 1;
                self->stored_length = self->cur_index - self->split_start_index;
                self->is_backward = 1;
                self->cur_index -= 1;
            }
            swap_cells(a, cell);
        } else if (self->is_backward && string->id == self->imprint && string->index == self->cur_index) {
            if (string->character == ' ') {
                self->stored_index = string->length;
                self->stored_length = string->length;
                self->destination_id = randrange(1, 256);
                clear_cell(a);
                a->type = RES;
            } else {
                string->length = self->stored_length;
                string->id = self->destination_id;
                string->index = self->stored_index;
            }
            if (self->cur_index == 0) {
                clear_cell(cell);
                cell->type = RES;
                return;
            }
            self->stored_index -= 1;
            self->cur_index -= 1;
            swap_cells(a, cell);
        }
    } else if (self->imprint == 0) {
        diffuse(cell, 1);
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
            if (string_id == cell->data[1]) {
                cell->data[2] = 0; // reset death clock
                a->type = FALLER;
            } else if (string_id == cell->data[0]) {
                cell->data[2] = 0; // reset death clock
                a->type = RISER;
            }
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

