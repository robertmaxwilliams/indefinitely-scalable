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

typedef struct {
    char type;
    char steps;
    char rot;
    char depth; // 0 is middle, 1 is outer, 2 is inner
    char size;
} thick_box_t;

#ELEMENT THICK_BOX 0x245A31 // box, dark green
void update_thick_box(cell_t* cell) {
    thick_box_t* self = (void*) cell;

    PATTERN ROTATE self->rot
        |  rxr
        |  ror
        | rb.ar
        |  rir
        |   r; 
    if (self->size == 0) {
        self->size = 10;
    }
    if (self->steps >= self->size) {
        // programmed death if in an unspecified state
        clear_cell(cell);
        cell->type = RES;
        return;
    }

    if (r->type == RES) {
        if (is_empty(a)) {
            r->type = BLANK;
            a->type = THICK_BOX;
            thick_box_t* a_box = (void*) a;
            a_box->depth = self->depth;
            a_box->size = self->size;
            if (self->steps == self->size - 1) {
                a_box->steps = 0;
                a_box->rot = (self->rot + 1) % 4;
            } else {
                a_box->steps = self->steps + 1;
                a_box->rot = self->rot;
            }
        } else if (is_empty(b) && self->steps != 0) { // TODO linear leaves weak spot
            r->type = BLANK;
            b->type = THICK_BOX;
            thick_box_t* b_box = (void*) b;
            b_box->steps = self->steps - 1;
            b_box->rot = self->rot;
            b_box->depth = self->depth;
            b_box->size = self->size;
        } else if (is_empty(o) && self->steps != 0 && self->depth == 0) {
            r->type = BLANK;
            o->type = THICK_BOX;
            thick_box_t* o_box = (void*) o;
            o_box->steps = self->steps + 1;
            o_box->rot = self->rot;
            o_box->depth = 1;
            o_box->size = self->size + 2;
        } else if (is_empty(i) && self->steps > 1 && self->steps < self->size - 2 && self->depth == 0) {
            r->type = BLANK;
            i->type = THICK_BOX;
            thick_box_t* i_box = (void*) i;
            i_box->steps = self->steps - 1;
            i_box->rot = self->rot;
            i_box->depth = 2;
            i_box->size = self->size - 2;
        }
    } else if (r->type == DREG) {
        swap_cells(x, r);
    }
}

#ELEMENT BOX_EATER
void update_box_eater(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    if (a->type == BOX) {
        a->type = BOX_EATER;
    }
    if (randp(100)) {
        clear_cell(cell);
        cell->type = RES;
    }
}

typedef struct {
        char type;
        char orientation; //data[0]
} inner_boxer_t;
#ELEMENT INNER_BOXER
void update_inner_boxer(cell_t* cell) {
    PATTERN ROTATE cell->data[0] % 4
        |   aaxaa
        |   bb.bb
        |   ccycc;
    if (a->type == INNER_BOXER) {
        swap_cells(x, cell);
    } else if (c->type == INNER_BOXER) {
        swap_cells(y, cell);
    } else if (c->type == OUTER_BOXER) {
        cell->data[0] += 2;
    }
}



typedef struct {
        char type;
        char orientation; //data[0]
} outer_boxer_t;
#ELEMENT OUTER_BOXER
void update_outer_boxer(cell_t* cell) {
    PATTERN ROTATE cell->data[0] % 4
        |   aaxaa
        |   bb.bb
        |   ccycc;
    if (a->type == OUTER_BOXER) {
        swap_cells(x, cell);
    } else if (c->type == OUTER_BOXER) {
        swap_cells(y, cell);
    } else if (a->type == INNER_BOXER) {
        cell->data[0] += 2;
    }
}
