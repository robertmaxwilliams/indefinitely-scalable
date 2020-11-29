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
        if (cell->data[0] <= new_data) {
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
            | aa aa
            | aa.aa
            | aa aa;
        unsigned char* openness = &cell->data[2];
        unsigned char* crowdedness = &cell->data[3];
        if (a->type == SORTER) {
            *openness = 0;
            *crowdedness += 1;
            if (*crowdedness > 4) {
                // programmed cell death
                clear_cell(cell);
                return;
            }
            diffuse(cell, 1);
        } else {
            *openness += 1;
            *crowdedness = 0;
            if (*openness > 200) {
                clear_and_set_type_if_cell_is_empty(random_neighbor(cell), SORTER);
                *openness = 0;
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
        a->type = DATA_EATER;
        a->data[1] = cell->data[1] - 1;
    }

    if (x->type == DATA) {
        cell->data[0] = x->data[0];
        clear_cell(x);
        fprintf(outfile, "%d, %d\n", cell->data[0], cell->data[1]);
    }
}
