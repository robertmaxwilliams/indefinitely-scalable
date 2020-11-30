
/*
 * DO NOT add any elements above BLANK or STRING.
 * These elements are the basic ones that make up the background/vacuum state of the world.
 */

/* 
 * CELL FUNC AREA
 *
 * cells:
 * isolater i
 * string s
 * dreg d
 * res r
 */
#ELEMENT BLANK 0xA9CCE3 // blank/background 0 sky blue
void update_blank(cell_t* cell) {
    (void)(cell);
}

// data is {id, string index, character, length}
#ELEMENT STRING 0x95A5A6  // grey color
void update_string(cell_t* cell) {
    diffuse(cell, 6);
}

#ELEMENT FROZEN_STRING
void update_frozen_string(cell_t* cell) {
    (void)cell;
}

#ELEMENT DREG 0xD35400 // orange color
void update_dreg(cell_t* cell) {
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa
        |   a;
    if (a == hole)
        return;

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

        case HWALL:
            break;
        case DATA_EATER:
            break;
        case DATA_EMITTER:
            break;
        case WALL:
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

#ELEMENT RES 0x58D68D  // lime color
void update_res(cell_t* cell) {
    diffuse(cell, 3);
}


#ELEMENT WALL 0x1B2631 // wall black
void update_wall(cell_t* cell) {
    (void)(cell);
}

