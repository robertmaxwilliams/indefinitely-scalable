
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

#ELEMENT INTIALIZER 0x000000
void update_initializer(cell_t* cell) {
    PATTERN
        |  xxx
        | xa.xx
        |  xxx;
    a->type = SALES;

    b->type = CITY;
    city_t* bb = (void*) b;
    bb->id = 0;
    bb->x = 4.23;
    bb->y = 


typedef struct {
    char type;
    char id;
    float x;
    float y;
} city_t;

#ELEMENT CITY 0xffff00
void update_city(cell_t* cell) {
    PATTERN ROTATE rand()
        | .xxx;
    if (is_empty(x)) {
        swap_cells(x, cell);
    }
}


typedef struct {
    char type;
    char id;
    float x;
    float y;
    char is_inactive;
} frozen_city_t;

#ELEMENT FROZEN_CITY 0x0000ff
void update_frozen_city(cell_t* cell) {
    frozen_city_t* self = (void*) cell;
    PATTERN
        | x.;
    if (x->type == FROZEN_CITY && !self->is_inactive) {
        swap_cells(x, cell);
    } else if (is_empty(x)) {
        self->is_inactive = 1;
    }
}

typedef struct {
    char type;
    float traveled; // 4 bytes
    unsigned short visited; // 4 bytes?
} sales_t;

int has_been_visited(unsigned int visited, char id) {
    return ((1 << id) & visited);
}

unsigned int visit(unsigned int visited, char id) {
    return (visited | (1 << id));
}

float sq(float a) {
    return a*a;
}

#ELEMENT SALES 0xff0000
void update_sales(cell_t* cell) {
    sales_t* self = (void*) cell;
    // a is looking for city, o is last city.
    PATTERN
        |   aaa
        |   .xaa
        |   oaa;
    if (a->type == CITY) {
        // found a city
        city_t* new_city = (void*) a;
        city_t* old_city = (void*) o;

        printf("%d, %d, %d\n", new_city->id, self->visited, has_been_visited(self->visited, new_city->id));
        if (!has_been_visited(self->visited, new_city->id)) {
            if (is_empty(o)) {
                // bring it into the "old city" spot if it's empty
                a->type = FROZEN_CITY;
                swap_cells(o, a);
            } else if (o->type == FROZEN_CITY && is_empty(x)) {
                // add distance from old to new to our accumulator, put old in the snake and put new in the "old city"
                // spot
                a->type == FROZEN_CITY;
                self->visited = visit(self->visited, new_city->id);
                self->traveled += sqrtf(sq(new_city->x - old_city->x) + sq(new_city->y - old_city->y));
                swap_cells(cell, x);
                swap_cells(cell, o);
                swap_cells(o, a);
                /* Final configuration:
                 * xxx
                 * o.xx
                 * axx
                */
            }
        }
    }
}



