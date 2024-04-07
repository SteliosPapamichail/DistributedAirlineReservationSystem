#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "stack/stack.h"
#include "queue/queue.h"

struct flight_reservations {
    struct stack *completed_reservations;
    struct queue *pending_reservations;
};

struct airline_company {
    pthread_t thread;
    unsigned int id;
};

/**
 * An agency produces A reservations in total, with res numbers:
 * i*A^2 + id
 */
struct agency {
    pthread_t thread;
    unsigned int id;
};

/**
 * The code to run when an airline company thread is spawned
 * @param args
 * @return
 */
void *airline_main(void *args) {
    //TODO:sp
    return NULL;
}

/**
 * The code to run when an agency thread is spawned
 * @param args
 * @return
 */
void *agency_main(void *args) {
    //TODO:sp
    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc > 2) exit(-1);

    // read a physical number and convert to int
    int A = atoi(argv[1]);

    // declare the airliner, flights and agency threads
    struct airline_company *airlineCompanies[A];
    struct agency *agencies[A * A];
    struct flight_reservations *flights[A]; // reservation i belongs to airline with id (i + 1)

    for (int i = 0; i < A * A; i++) {
        if (i < A) {
            // init airline threads
            airlineCompanies[i] = (struct airline_company *) malloc(sizeof(struct airline_company));
            airlineCompanies[i]->id = i + 1;
            pthread_create(&(airlineCompanies[i]->thread), NULL, airline_main, NULL);
            // init flight reservations table
            flights[i] = (struct flight_reservations *) malloc(sizeof(struct flight_reservations));
            // stack capacity depends on the flight's position in the table
            flights[i]->completed_reservations = createStack((3 / 2) * (A ^ 2) - (A - 1 - i) * A);
            flights[i]->pending_reservations = createQueue();
        }
        // init agencies
        agencies[i] = (struct agency *) malloc(sizeof(struct agency));
        agencies[i]->id = i + 1;
        pthread_create(&(agencies[i]->thread), NULL, agency_main, NULL);
    }

    //todo cleanup flights, companies, agencies, etc.
}