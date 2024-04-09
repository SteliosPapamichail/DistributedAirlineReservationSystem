#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include "stack/stack.h"
#include "queue/queue.h"
#include "common/reservations.h"
#include "list/lazy_list.h"


/**
 * Will be equal to A^2 agencies
 */
unsigned int numOfAgencies = 0;

/**
 * Will be equal to A flights
 */
unsigned int numOfFlights = 0;

/**
 * Will be equal to A companies
 */
unsigned int numOfAirlineCompanies = 0;

/**
 * The flight controller responsible for validating flight reservations
 */
pthread_t flight_controller;

/**
 * Barrier that is responsible for guaranteeing that the flight controller
 * will start performing checks, only after all reservations have been added to
 * the system.
 */
pthread_barrier_t barrier_start_1st_phase_checks;

/**
 * Signals that the phase 1 checks have finished and phase 2 can begin.
 */
pthread_barrier_t barrier_start_2nd_phase;

/**
 * Signals that reservation management has finished and the phase 2
 * checks can begin.
 */
pthread_barrier_t barrier_start_2nd_phase_checks;

struct flight_reservations {
    struct stack *completed_reservations;
    struct queue *pending_reservations;
};

struct agency_args {
    int agency_id;
    struct flight_reservations *flight;
};

struct thread_args {
    unsigned int id;
};

struct flight_controller_args {
    int id;
    struct flight_reservations **flights;
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
    struct agency_args *th_args = (struct agency_args *) args; // cast args back to struct ptr
    // produce A reservations concurrently
    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct Reservation *reservation = (struct Reservation *) malloc(sizeof(struct Reservation));
        reservation->agency_id = th_args->agency_id;
        reservation->reservation_number = (i * numOfAgencies) + th_args->agency_id;
        if (isStackFull(th_args->flight->completed_reservations)) {  // add reservation to queue if stack is full
            enqueue(th_args->flight->pending_reservations, *reservation);
        } else { // add to stack
            push(th_args->flight->completed_reservations, *reservation);
        }
        free(reservation);
    }

    // agency has finished importing flights, should wait for all others
    pthread_barrier_wait(&barrier_start_1st_phase_checks);

    // agency is done, free up memory
    free(th_args);
    return NULL;
}

void *flight_controller_main(void *args) {
    pthread_barrier_wait(&barrier_start_1st_phase_checks); // wait for agencies
    struct flight_controller_args *controllerArgs = (struct flight_controller_args *) args;// cast to controller args
    // check-related vars
    unsigned int totalReservations = 0;
    unsigned int expectedTotalReservations = pow(numOfFlights, 3);
    unsigned int totalKeySum = 0;
    unsigned int expectedKeySum = (((pow(numOfFlights, 6)) + (pow(numOfFlights, 3))) / 2); // (A^6 + A^3) / 2

    // start checks and check-related calculations
    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct stack *completedReservations = controllerArgs->flights[i]->completed_reservations;
        struct queue *pendingReservations = controllerArgs->flights[i]->pending_reservations;

        // check #1: stack overflow
        if (hasStackOverflowed(completedReservations)) {
            // log the error and exit
            printf("Flight %d: stack has overflowed! Check failed (capacity: %u, found: %u)\n", i,
                   completedReservations->capacity,
                   completedReservations->size);
            pthread_exit((void *) -1); //todo:sp revisit so that the program exits but cleans up first
        }
        printf("Flight %d: stack overflow check passed (capacity: %u, found: %u)\n", i,
               completedReservations->capacity,
               completedReservations->size);
        // check #2: total size
        totalReservations += completedReservations->size + pendingReservations->size;
        // check #3: total keysum
        // traverse the stack and sum the reservation numbers
        struct stack_reservation *currentStackNode = completedReservations->top;
        // lock to ensure thread safety
        pthread_mutex_lock(&(completedReservations->top_lock));
        while (currentStackNode != NULL) {
            totalKeySum += currentStackNode->reservation.reservation_number;
            currentStackNode = currentStackNode->next;
        }
        pthread_mutex_unlock(&(completedReservations->top_lock));

        if (isStackFull(
                completedReservations)) { // traverse the queue if needed and sum the reservation numbers
            // traverse queue

            // acquire locks for thread safety
            pthread_mutex_lock(&pendingReservations->head_lock);
            pthread_mutex_lock(&pendingReservations->tail_lock);
            struct queue_reservation *curr = pendingReservations->head->next; // get first node by skipping dummy node
            while (curr != NULL) {
                totalKeySum += curr->reservation.reservation_number;
                curr = curr->next;
            }

            // update number of inserter airlines if the queue is not empty for this flight
            if (pendingReservations->size > 0) {
                number_of_inserter_airlines += 1;
            }

            // release locks
            pthread_mutex_unlock(&pendingReservations->head_lock);
            pthread_mutex_unlock(&pendingReservations->tail_lock);

        }
    }
    // check #2: total size
    if (totalReservations != expectedTotalReservations) {
        printf("Total size check failed (expected: %d, found: %d)\n", expectedTotalReservations, totalReservations);
        pthread_exit((void *) -2);
    }
    printf("Total size check passed (expected: %d, found: %d)\n", expectedTotalReservations, totalReservations);

    // check #3: Total keysum
    if (totalKeySum != expectedKeySum) {
        printf("Total keysum check failed (expected: %d, found %d)\n", expectedKeySum, totalKeySum);
        pthread_exit((void *) -3);
    }
    printf("Total keysum check passed (expected: %d, found: %d)\n", expectedKeySum, totalKeySum);
    // all checks passed for phase 1

    // wait for companies to start phase 2
    pthread_barrier_wait(&barrier_start_2nd_phase);

    //todo:sp implement

    // wait for companies to finish processing reservations before starting phase 2 checks
    pthread_barrier_wait(&barrier_start_2nd_phase_checks);
    //todo:sp implement

    free(controllerArgs);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) exit(-1);

    // read a physical number and convert to int
    int A = atoi(argv[1]);

    // declare the airliner, flights and agency threads
    pthread_t airlineCompanies[A];
    numOfFlights = A;
    numOfAirlineCompanies = A;
    numOfAgencies = A * A;
    pthread_t agencies[numOfAgencies];
    struct agency_args *agencyArguments[numOfAgencies];
    struct flight_reservations *flights[A]; // reservation i belongs to airline with agency_id (i + 1)

    // init controller barrier for phase 1 checks
    pthread_barrier_init(&barrier_start_1st_phase_checks, NULL, numOfAgencies + 1); // Π agencies plus the controller
    // init phase 2 barrier for airline companies and the controller
    pthread_barrier_init(&barrier_start_2nd_phase, NULL, numOfAirlineCompanies + 1);
    // init controller barrier for phase 2 checks
    // airline companies will wait before termination at this barrier and the controller can proceed with the checks after waiting on this barrier
    pthread_barrier_init(&barrier_start_2nd_phase_checks, NULL, numOfAirlineCompanies + 1);

    // init inserter airlines lock
    pthread_mutex_init(&inserter_airlines_lock, NULL);

    for (int i = 0; i < A * A; i++) {
        if (i < A) {
            // init flight reservations table
            flights[i] = (struct flight_reservations *) malloc(sizeof(struct flight_reservations));
            // stack capacity depends on the flight's position in the table
            unsigned int capacity = (3 / 2) * (A * A) - (A - 1 - i) * A;
            flights[i]->completed_reservations = createStack(capacity);
            flights[i]->pending_reservations = createQueue();
        }
        // init agencies
        agencyArguments[i] = malloc(sizeof(struct agency_args));
        agencyArguments[i]->agency_id = i + 1;
        agencyArguments[i]->flight = flights[i % A]; // the flight for whose reservations the agency is responsible
        pthread_create(&(agencies[i]), NULL, agency_main, agencyArguments[i]);
    }


    // init the flight controller
    struct flight_controller_args *controllerArgs = malloc(sizeof(struct flight_controller_args));
    controllerArgs->id = 0;
    controllerArgs->flights = flights;
    pthread_create(&flight_controller, NULL, flight_controller_main, controllerArgs);

    // wait for controller and agency threads to finish
    for (unsigned int i = 0; i < numOfAgencies; i++) {
        pthread_join(agencies[i], NULL);
    }
    pthread_join(flight_controller, NULL);

    pthread_barrier_destroy(&barrier_start_1st_phase_checks);

    //todo cleanup flights, companies, agencies, etc.
}