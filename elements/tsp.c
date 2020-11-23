
/* 
 * Cell types for solving TSP Problems
 * City stores its coordinates and floats around
 * SalesPerson sits still and builds up a snake of Data elements with cities,
 *   and only keeps cities it hasn't seen
 * Delegate is emited by a complete city and resets inferior completed SalesPersons
 * Printer should be placed when you want a solution output. It writes to tsp-solutions.txt
 * and writes the best solution it can find.
 * Intializer  puts evertything out to start the problem.
 * And don't forget - each cell can only hold 10 bytes!
*/

city_el_t cities[] = {{0, 3.21, 5.42},
                            {1, 6.32, 8.41},
                            {2, 7.42, 1.62},
                            {3, 5.37, 7.44},
                            {4, 6.48, 2.26},
                            {5, 2.84, 8.62},
                            {6, 7.18, 2.71}};
#define N_CITIES 7
#define FULL_BIN 0b1111111

#ELEMENT CITY_EMITTER 0x111111
void update_city_emitter(cell_t* cell) {
    PATTERN
        | .a b;
    if (randp(50) && is_empty(a)) {
        a->type = CITY;
        city_el_t city = cities[rand()%N_CITIES];
        city_t* aa = (void*) a;
        aa->id = city.id;
        aa->x = city.x;
        aa->y = city.y;
    } else if (randp(250)) {
        clear_cell(a);
        a->type = PROTO_SALESPERSON;
    }

    if (randp(50) && b->type == CITY) {
        clear_cell(b);
    }
}


#ELEMENT CITY 0xffff00
void update_city(cell_t* cell) {
    PATTERN ROTATE rand()
        | .xxx;
    if (is_empty(x)) {
        swap_cells(x, cell);
    }
}


#ELEMENT FROZEN_CITY 0x0000ff
void update_frozen_city(cell_t* cell) {
    frozen_city_t* self = (void*) cell;
    PATTERN
        | x.;
    frozen_city_t* left = (void*) x;
    if (x->type == FROZEN_CITY && !self->is_inactive && left->is_inactive) {

        swap_cells(x, cell);
    } else if (is_empty(x)) {
        self->is_inactive = 1;
    }
}

typedef struct {
    char type;
    float traveled; // 4 bytes
    unsigned short visited; // 4 bytes?
} salesperson_t;

int has_been_visited(unsigned int visited, char id) {
    return ((1 << id) & visited);
}

unsigned int visit(unsigned int visited, char id) {
    return (visited | (1 << id));
}

float sq(float a) {
    return a*a;
}

void move_city_to_frozen_city(frozen_city_t* old_city, city_t* new_city) {
    old_city->type = FROZEN_CITY;
    old_city->id = new_city->id;
    old_city->x = new_city->x;
    old_city->y = new_city->y;
    old_city->is_inactive = 0;
}

#ELEMENT PROTO_SALESPERSON 0x990000
void update_proto_salesperson(cell_t* cell) {
    if (randp(1000)) {
        clear_cell(cell);
        return;
    }
    PATTERN
        |   a
        |  aaa
        | aa.aa
        |  aaa 
        |   a;
    if (a->type == SALESPERSON) {
        cell->data[0] = 0;
    } else {
        cell->data[0] += 1;
    }

    if (cell->data[0] == 255) {
        clear_cell(cell);
        cell->type = SALESPERSON;
    } else if (is_empty(a)) {
        swap_cells(a, cell);
    }
}

#ELEMENT SALESPERSON 0xff0000
void update_salesperson(cell_t* cell) {
    // Guard in case it runs into another salesperson's data
    {
        PATTERN
            |  a
            |x.b
            | yc;
        if (c == hole || b == hole || b->type == CITY_EMITTER || a->type == FROZEN_CITY || b->type == FROZEN_CITY || c->type == FROZEN_CITY) {
            clear_cell(cell);
            clear_cell(x);
            clear_cell(y);
            cell->type = PROTO_SALESPERSON;
            x->type = SALESPERSON_EATER;
            return;
        }
    }

    // begin normal code
    salesperson_t* self = (void*) cell;
    // a is looking for city, o is last city.
    PATTERN
        |   aaa
        |   .xaa
        |   oga;
    if (o->type == FROZEN_CITY && self->visited == FULL_BIN && is_empty(x)) {
        // if we're done, straighten up
        swap_cells(cell, x);
        swap_cells(cell, o);
        return;
    }

    if (a->type == CITY) {
        // found a city
        city_t* new_city = (void*) a;
        frozen_city_t* old_city = (void*) o;

        if (!has_been_visited(self->visited, new_city->id)) {
            if (is_empty(o)) {
                // bring it into the "old city" spot if it's empty
                move_city_to_frozen_city(old_city, new_city);
                self->visited = visit(self->visited, new_city->id);
                clear_cell(a);
            } else if (o->type == FROZEN_CITY && is_empty(x)) {
                // add distance from old to new to our accumulator, put old in the snake and put new in the "old city"
                // spot
                self->traveled += sqrtf(sq(new_city->x - old_city->x) + sq(new_city->y - old_city->y));
                self->visited = visit(self->visited, new_city->id);
                swap_cells(cell, x);
                swap_cells(o, cell);
                frozen_city_t* new_old_city = (void*) g;
                move_city_to_frozen_city(new_old_city, new_city);

                /* Final configuration:
                 * xxx
                 * o.xx
                 * gax
                */
            }
        }
    }

}

typedef struct {
    char type;
    char initialized;
    char sequence_mode;
    float min_distance; // 4 bytes
} result_printer_t;

#ELEMENT RESULT_PRINTER 0xff00ff
void update_result_printer(cell_t* cell) {
    result_printer_t* self = (void*) cell;

    if (self->sequence_mode) {
        PATTERN
            |x
            | g.
            |  b;
        // If we see a salesperson, we haven't started yet and need to get
        // in position
        if (g->type == SALESPERSON) {
            if (is_empty(x)) {
                swap_cells(cell, x);
            }
            return;
        }

        // abort if we have nowhere to go
        if (!is_empty(g)) {
            return;
        }

        // print out each city as it shows up below
        // TODO deal with overlapping printing issues
        if (b->type == FROZEN_CITY) {
            frozen_city_t* city = (void*) b;
            printf("[%d]", city->id);
            swap_cells(cell, g);
        } else {
            printf("\n");
            self->sequence_mode = 0;
        }
        return;
    }

    {   // If you meet an initialized Result Printer, give us both the min of our min_distances
        PATTERN
            | aaa
            | a.a
            | aaa;
        result_printer_t* other = (void*) a;
        if (other->type == RESULT_PRINTER && other->initialized && self->initialized) {
            if (other->min_distance < self->min_distance) {
                self->min_distance = other->min_distance;
            } else {
                other->min_distance = self->min_distance;
            }
            return;
        }
    }

    PATTERN
        | ya.
        |  x;
    salesperson_t* sales = (void*) a;
    if (!self->initialized) {
        self->initialized = 1;
        self->min_distance = 10000;
    }
    if (sales->type == SALESPERSON) {
        if (sales->visited == FULL_BIN) {
            if (sales->traveled < self->min_distance) {
                self->min_distance = sales->traveled;
                printf("%f is the best distance.\n", self->min_distance);
                self->sequence_mode = 1;
                return;
            } else if (sales->traveled > self->min_distance) {
                clear_cell(a);
                clear_cell(x);
                clear_cell(y);
                a->type = PROTO_SALESPERSON;
                y->type = SALESPERSON_EATER;
            }
        }
    }
    diffuse(cell, 1);
}

#ELEMENT SALESPERSON_EATER 0x00ffff
void update_salesperson_eater(cell_t* cell) {
    PATTERN
        |  a
        | a.a
        |  a;
    if (a->type == SALESPERSON || a->type == FROZEN_CITY) {
        clear_cell(a);
        a->type = SALESPERSON_EATER;
    }

    if (randp(10)) {
        cell->data[0] += 1;
    }

    if (cell->data[0] > 50) {
        clear_cell(cell);
    }
}
