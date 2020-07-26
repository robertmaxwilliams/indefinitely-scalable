/*
 * Physical structures, like boxes and walls.
 */

// helper for update_rememberer
void remember_and_place(cell_t* cell, unsigned char i, cell_t* neigh, cell_t* maybe_res) {
    if (neigh->type == REMEMBERER) {
        cell->data[i%10] = 1;
    } else if (maybe_res->type == RES && cell->data[i%10] == 1) {
        swap_cells(neigh, maybe_res);
        clear_cell(neigh);
        neigh->type = REMEMBERER;
    }
}

// stores whether there is someone at various relative positions, 
// in the following way: {top, topright, right, rightbuttom, bottom, bl, left, lt, empty, rot}
#ELEMENT REMEMBERER 0x641E16 // rememberer burnt red
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
#ELEMENT BOX 0x145A32 // box, dark green
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
