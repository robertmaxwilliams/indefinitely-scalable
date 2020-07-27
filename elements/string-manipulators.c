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
#ELEMENT STRING_REVERSER 0xff0000
void update_string_reverser(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    // unsafe cast our cell and the string to a new type with better names
    struct {
        char type;
        char imprint;
        char boredom;
        char destination_id;
        char max_index;
        char saved_index;
        char saved_character;
        char has_one_saved;
    } * self = (void*) cell;

    struct { char type; char id; char index; char character; } * string = (void*) a;

    if (string->type == STRING) { 
        if (self->imprint == 0) { // if we aren't imprinted, imprint on this string
            self->imprint = string->id;
            self->destination_id = randrange(1, 256);
            self->max_index = string->index;
        } else if (string->id == self->imprint) { //if this str is our mark...
            if (self->boredom < 100) { // we're still looking
                if (string->index > self->max_index) {
                    self->max_index = string->index;
                    self->boredom = 0;
                } else if (randp(2)) {
                    // TODO a more sophisticated boredome algorithm, 
                    // based on approximate string size
                    self->boredom += 1;
                }
            } else { // we're done looking and can start reversing
                string->id = self->destination_id;
                string->index = self->max_index - string->index;
            }
        }
    }

    diffuse(cell, 1);
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

