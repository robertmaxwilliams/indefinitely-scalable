
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

city_el_t cities[] = {
    {0, 2.0, 3.5},
    {1, 1.0, 6.5},
    {2, 5.4, 6.12},
    {3, 4.2, 7.0},
    {4, 2.6, 6.22},
    {5, 3.7, 5.19},
    {6, 4.6, 5.7},
    {7, 4.7, 6.13},
    {8, 2.4, 4.5},
    {9, 4.3, 3.13},
    {10, 1.9, 3.22},
    {11, 2.2, 5.99}};

#define N_CITIES 12
#define FULL_BIN ((1U << N_CITIES) -1)
//0b11111

#ELEMENT CITY_EMITTER 0x111111
void update_city_emitter(cell_t* cell) {
    PATTERN
        |  c
        | c.c
        |  c;
    if (c->type == FROZEN_CITY || c->type == SALESPERSON) {
        clear_cell(c);
        c->type = SALESPERSON_EATER;
    }

    PATTERN
        | .a b;
    if (randp(30) && is_empty(a)) {
        //printf("%lu, %lu, %lu %lu %lu\n", sizeof(float), sizeof(short),
        //        sizeof(frozen_city_t), sizeof(city_t), sizeof(salesperson_t));
        a->type = CITY;
        city_el_t city = cities[rand()%(N_CITIES-1)+1];
        city_t* aa = (void*) a;
        aa->id = city.id;
        //printf("new city: %f %f\n", city.x, city.y);
        aa->x = city.x;
        aa->y = city.y;
    } else if (randp(100)) {
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
        |  a
        | x.b;
    frozen_city_t* left = (void*) x;
    // if we're abandoned, PCD
    if ((is_empty(a) && is_empty(b)) || b == hole || b->type == CITY_EMITTER) {
        clear_cell(cell);
        return;
    }
    if (x->type == FROZEN_CITY && !self->is_inactive && left->is_inactive) {

        swap_cells(x, cell);
    } else if (is_empty(x)) {
        self->is_inactive = 1;
    }
}

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

float city_distance(city_t* a, frozen_city_t* b) {
    return sqrtf(sq(a->x - b->x)
            + sq(a->y - b->y));
}


float city_distance2(frozen_city_t* a, frozen_city_t* b) {
    return sqrtf(sq(a->x - b->x)
            + sq(a->y - b->y));
}
float city_distance_from_zero(frozen_city_t* city) {
    return sqrtf(sq(city->x - cities[0].x)
            + sq(city->y - cities[0].y));
}

void finish_city(cell_t* cell) {
    salesperson_t* self = (void*) cell;
    PATTERN
        |   .x
        |   o;
    if (self->type != SALESPERSON) {
        printf("ooops\n");
    }
    if (!(o->type == FROZEN_CITY && self->visited == FULL_BIN)) {
        printf("ooops2\n");
    }

    // if we're done, straighten up and add distance back to city 0
    frozen_city_t* final_city = (void*) o;
    self->is_done = 1;
    self->traveled += city_distance_from_zero(final_city);
    swap_cells(cell, x);
    swap_cells(cell, o);
    return;
}

#ELEMENT SALESPERSON 0xff0000
void update_salesperson(cell_t* cell) {
    // small chance of programmed cell death
    if (randp(100000)) {
        clear_cell(cell);
        cell->type = PROTO_SALESPERSON;
        return;
    }
    salesperson_t* self = (void*) cell;
    // quick hack since we remove city 0
    self->visited |= 1U;

    // Guard in case it runs into another salesperson's data
    {
        PATTERN
            |  a
            | .b
            |  c;
        PATTERN
            |ooo
            |o.o
            |ooo;
        if (
                (a == hole || (!is_empty(a) && a->type != CITY && a->type != RESULT_PRINTER && a->type != PROTO_SALESPERSON))
                || (b == hole || (!is_empty(b) && b->type != CITY && b->type != RESULT_PRINTER && b->type != PROTO_SALESPERSON))
                || (c == hole || (!is_empty(c) && c->type != CITY && c->type != RESULT_PRINTER && c->type != PROTO_SALESPERSON))) {
            clear_cell(cell);
            cell->type = PROTO_SALESPERSON;
            return;
        }

    }

    // begin normal code
    // a is looking for city, o is last city.
    PATTERN
        |   aaa
        |   .xaa
        |   oga;
    if (a->type == CITY) {
        // found a city
        city_t* new_city = (void*) a;
        frozen_city_t* old_city = (void*) o;

        if (!has_been_visited(self->visited, new_city->id)) {
            if (is_empty(o)) {
                // bring it into the "old city" spot if it's empty
                move_city_to_frozen_city(old_city, new_city);
                self->visited = visit(self->visited, new_city->id);
                self->traveled += city_distance_from_zero(old_city);
                clear_cell(a);
                return;
            } else if (o->type == FROZEN_CITY && is_empty(x)) {
                // add distance from old to new to our accumulator, put old in the snake and put new in the "old city"
                // spot
                self->visited = visit(self->visited, new_city->id);
                self->traveled += city_distance(new_city, old_city);
                int is_done = (self->visited == FULL_BIN);
                swap_cells(cell, x);
                swap_cells(o, cell);
                frozen_city_t* new_old_city = (void*) g;
                move_city_to_frozen_city(new_old_city, new_city);

                /* Final configuration:
                 * xxx
                 * o.xx
                 * gax
                */
                if (is_done) {
                    finish_city(x);
                }
                return;
            }
        }
    }


}

typedef struct {
    char type;
    float min_distance; // 4 bytes
    char initialized;
    char sequence_mode;
} result_printer_t;

int is_first_time = 1;
clock_t start;
#ELEMENT RESULT_PRINTER 0xff00ff
void update_result_printer(cell_t* cell) {
    if (is_first_time) {
        is_first_time = 0;
        start = clock();
    }

    if (randp(100)) {
        PATTERN
            | a.a;
        if (is_empty(a)) {
            copy_cell(a, cell);
        }
    }



    result_printer_t* self = (void*) cell;

    if (self->sequence_mode) {
        if (randp(1000)) {
            // sometimes we get stuck here, so occasionally die
            clear_cell(cell);
        }

        PATTERN
            |x
            |cg.
            | nbp;
        // If we see a salesperson, we haven't started yet and need to get
        // in position
        if (g->type == SALESPERSON) {
            if (is_empty(x)) {
                printf("[0] %f ", city_distance_from_zero((frozen_city_t*) c));
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
            if (n->type == FROZEN_CITY)
                printf(" %f ", city_distance2((frozen_city_t*) b, (frozen_city_t*) n));
            else
                printf(" %f [0]", city_distance_from_zero((frozen_city_t*) b));

                
            swap_cells(cell, g);
        } else {
            p->type = SALESPERSON_EATER;
            printf("\n");
            self->sequence_mode = 0;
        }
        return;
    }

    {   // If you meet an initialized Result Printer, copy it's min distance if better or kill it
        PATTERN
            | aaa
            | a.a
            | aaa;
        result_printer_t* other = (void*) a;

        if (other->type == RESULT_PRINTER && other->initialized && self->initialized) {
            if (other->min_distance < self->min_distance) {
                if (randp(100)) {
                    clear_cell(a);
                } else {
                    self->min_distance = other->min_distance;
                }
            } else {
                clear_cell(a);
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
        if (sales->is_done) {
            if (sales->traveled < self->min_distance) {
                self->min_distance = sales->traveled;
                printf("%f, %f\n", ((float)(clock()-start))/CLOCKS_PER_SEC, self->min_distance);
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
