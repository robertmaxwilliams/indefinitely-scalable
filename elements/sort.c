FILE* outfile = NULL;

#ELEMENT DATA 0x888888
void update_data(cell_t* cell) {
    diffuse(cell, 5);
}

#ELEMENT HWALL 0x030507
void update_hwall(cell_t* cell) {
    PATTERN
        |  x.x;
    if (is_empty(x)) {
        x->type = HWALL;
    }
}

// has n percent chance to return random result instead of true result
int faulty_leq(unsigned char a, unsigned char b, int percent_failure) {
    if (rand()%100 < percent_failure)
        return rand()%2;
    return a <= b;
}
// data[0] is stored value, data[1] is is_initialized,
// data[2] counts how many time we've diffused
#ELEMENT SORTER 0xffffff
void update_sorter(cell_t* cell) {
    PATTERN
        |  xx aa
        |  xx aa
        |    .
        |  yy aa
        |  yy aa;
    // randomly initialize self.data[0]
    if (cell->data[1] == 0) {
        cell->data[1] = 1;
        cell->data[0] = rand() % 256;
    }

    if (a->type == DATA) {
        // do the sorting
        unsigned char new_data = a->data[0];
        if (faulty_leq(cell->data[0], new_data, 25)) {
            if (is_empty(y)) {
                move_to_target_if_empty(y, a);
                cell->data[0] = new_data;
            }
        } else {
            if (is_empty(x)) {
                move_to_target_if_empty(x, a);
                cell->data[0] = new_data;
            }
        }
    } else {
        // or try to diffuse away from other sorters to fill space
        PATTERN
            | aaaaa
            | aa.aa
            | aaaaa;
        unsigned char* openness = &cell->data[2];
        unsigned char* crowdedness = &cell->data[3];
        if (a->type == SORTER) {
            diffuse(cell, 1);
        } else if (a->type == RES) {
            clear_cell(a);
            // only sometimes reproduce, to keep sparse enough not
            // to thrash so hard
            if (randp(2)) {
                a->type = SORTER;
            }
        }
    }
}

#ELEMENT DATA_EMITTER 0x9999ff
void update_data_emitter(cell_t* cell) {
    PATTERN
        |  a
        | x.
        |  a;
    if (is_empty(a)) {
        a->type = DATA_EMITTER;
    }

    if (is_empty(x) && randp(100)) {
        x->type = DATA;
        x->data[0] = rand() % 256;
    }
}

#ELEMENT DATA_EATER 0xff9999
void update_data_eater(cell_t* cell) {
    if (outfile == NULL) {
        outfile = fopen("sorting.csv", "w");
    }
    PATTERN
        | b
        | .xxx
        | a;
    if (is_empty(a)) {
        a->type = DATA_EATER;
        a->data[1] = cell->data[1] + 1;
    }
    if (is_empty(b)) {
        b->type = DATA_EATER;
        b->data[1] = cell->data[1] - 1;
    }

    if (x->type == DATA) {
        cell->data[0] = x->data[0];
        clear_cell(x);
        fprintf(outfile, "%d, %d\n", cell->data[0], cell->data[1]);
    }
}
