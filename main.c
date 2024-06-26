#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include "stack/stack.h"
#include "queue/queue.h"
#include "common/reservations.h"
#include "list/lazy_list.h"


pthread_mutex_t inserter_airlines_lock;

/**
 * Set by the flight controller and altered by airline companies (shared var)
 */
unsigned int number_of_inserter_airlines;


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

/**
 * Represents an agency's arguments that are passed to and used by agency threads
 */
struct agency_args {
    int agency_id;
    struct flight_reservations *flight;
};

/**
 * Represents the flight controller's arguments that are passed to and used by its thread
 */
struct flight_controller_args {
    int id;
    struct flight_reservations **flights;
    struct list *management_center;
};

/**
 * Represents an airline's arguments that are passed to and used by airline threads
 */
struct airline_args {
    struct flight_reservations *flight; // flight for which the company is responsible
    struct list *management_center;
};

/**
 * The code to run when an airline company thread is spawned
 * @param args Must be of type (struct airline_args *)
 * @return NULL if the thread completed its execution successfully
 */
void *airline_main(void *args) {
    // guarantee that phase 2 starts after controller finishes phase 1 checks
    pthread_barrier_wait(&barrier_start_2nd_phase);
    // cast back to args
    struct airline_args *airline_comp_args = (struct airline_args *) args;
    if (airline_comp_args->flight->pending_reservations->size > 0) { // if company has reservations in queue
        // move reservations from pending queue to the reservation center
        struct queue *pending_reservations = airline_comp_args->flight->pending_reservations;
        while (pending_reservations->size > 0) {
            struct Reservation reservation = dequeue(pending_reservations);
            if (reservation.reservation_number != -1) {
                insert(airline_comp_args->management_center, reservation);
            }
        }
        // update shared variable for inserter airlines
        pthread_mutex_lock(&inserter_airlines_lock);
        number_of_inserter_airlines--;
        pthread_mutex_unlock(&inserter_airlines_lock);
    } else if (!isStackFull(
            airline_comp_args->flight->completed_reservations)) { // if company has no pending reservations and has space on its stack
        struct stack *completed_reservations = airline_comp_args->flight->completed_reservations;

        // reservation transfers should happen until the stack is either full or the center is empty and inserter == 0
        while (!isStackFull(completed_reservations) &&
               (!isListEmpty(airline_comp_args->management_center) || number_of_inserter_airlines != 0)) {
            // move reservation to the stack from the center
            struct Reservation reservation = deleteAndGet(airline_comp_args->management_center);
            if (reservation.reservation_number != -1) push(completed_reservations, reservation);
        }

    }
    // signal to the controller that checks can start if all airliners have reached this point
    pthread_barrier_wait(&barrier_start_2nd_phase_checks);
    free(airline_comp_args);
    return NULL;
}

/**
 * The code to run when an agency thread is spawned
 * @param args Must be of type (struct agency_args *)
 * @return NULL if the thread completed its execution successfully
 */
void *agency_main(void *args) {
    struct agency_args *agency_args = (struct agency_args *) args; // cast args back to struct ptr
    // produce A reservations concurrently
    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct Reservation *reservation = (struct Reservation *) malloc(sizeof(struct Reservation));
        reservation->agency_id = agency_args->agency_id;
        reservation->reservation_number = (i * numOfAgencies) + agency_args->agency_id;
        if (isStackFull(agency_args->flight->completed_reservations)) {  // add reservation to queue if stack is full
            enqueue(agency_args->flight->pending_reservations, *reservation);
        } else { // add to stack
            push(agency_args->flight->completed_reservations, *reservation);
        }
        free(reservation);
    }

    // agency has finished importing flights, should wait for all others
    pthread_barrier_wait(&barrier_start_1st_phase_checks);

    // agency is done, free up memory
    free(agency_args);
    return NULL;
}

/**
 * Performs a stack overflow check for each given flight's completed
 * reservations.
 * @param flights An array of flights to check
 * @return 1 if successful, 0 otherwise
 */
int check_stack_overflow(struct flight_reservations **flights) {
    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct stack *completedReservations = flights[i]->completed_reservations;

        if (hasStackOverflowed(completedReservations)) {
            // log the error and exit
            printf("Flight %d: stack has overflowed! Check failed (capacity: %u, found: %u)\n", i,
                   completedReservations->capacity,
                   completedReservations->size);
            return 0;
        } else {
            printf("Flight %d: stack overflow check passed (capacity: %u, found: %u)\n", i,
                   completedReservations->capacity,
                   completedReservations->size);
        }
    }
    return 1;
}

/**
 * Performs a total size check for all given flights
 * by summing their completed & pending reservations.
 * @param flights An array of flights to check
 * @return 1 if successful, 0 otherwise
 */
int check_total_size(struct flight_reservations **flights) {
    unsigned int totalReservations = 0;
    unsigned int expectedTotalReservations = pow(numOfFlights, 3);

    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct stack *completedReservations = flights[i]->completed_reservations;
        struct queue *pendingReservations = flights[i]->pending_reservations;
        totalReservations += completedReservations->size + pendingReservations->size;
    }
    int result = totalReservations == expectedTotalReservations;
    if (!result) {
        printf("Total size check failed (expected: %d, found: %d)\n", expectedTotalReservations, totalReservations);
    } else {
        printf("Total size check passed (expected: %d, found: %d)\n", expectedTotalReservations, totalReservations);
    }
    return result;
}

/**
 * Performs a total keysum check for all given flights
 * by summing their completed & pending reservation numbers.
 * @param flights An array of flights to check
 * @return 1 if successful, 0 otherwise
 */
int check_total_keysum(struct flight_reservations **flights) {
    unsigned long totalKeySum = 0;
    unsigned long expectedKeySum = (((pow(numOfFlights, 6)) + (pow(numOfFlights, 3))) / 2); // (A^6 + A^3) / 2
    for (unsigned int i = 0; i < numOfFlights; i++) {
        struct stack *completedReservations = flights[i]->completed_reservations;
        struct queue *pendingReservations = flights[i]->pending_reservations;

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

    int result = totalKeySum == expectedKeySum;
    if (!result) {
        printf("Total keysum check failed (expected: %lu, found %lu)\n", expectedKeySum, totalKeySum);
    } else {
        printf("Total keysum check passed (expected: %lu, found: %lu)\n", expectedKeySum, totalKeySum);
    }
    return result;
}

/**
 * Performs a reservations completion check for all given flights and the
 * reservation management center by checking that all flight pending queues are empty
 * and that there are no more flights left in the center.
 * @param flights An array of flights to check
 * @param management_center The reservations center to check
 * @return 1 if successful, 0 otherwise
 */
int reservations_completion_check(struct flight_reservations **flights, struct list *management_center) {
    int result = 1;
    if (!isListEmpty(management_center)) {
        printf("Reservations center was not empty!\n");
        result = 0;
    } else if (number_of_inserter_airlines != 0) {
        printf("Number of inserter airlines was not zero!\n");
        result = 0;
    } else {
        // check all queues
        for (int i = 0; i < numOfFlights; i++) {
            struct queue *pendingReservations = flights[i]->pending_reservations;
            if (pendingReservations->size != 0) {
                printf("Found non empty pending reservations queue for flight #%d\n", i);
                result = 0;
            }
        }
    }
    if (result) printf("Reservations completion check passed\n");
    return result;
}

/**
 * The code to run when the flight controller thread is spawned. This
 * thread is responsible for synchronising and performing checks for both phases.
 * @param args Must be of type (struct flight_controller_args *)
 * @return NULL if the thread completed its execution successfully
 */
void *flight_controller_main(void *args) {
    pthread_barrier_wait(&barrier_start_1st_phase_checks); // wait for agencies
    struct flight_controller_args *controllerArgs = (struct flight_controller_args *) args;// cast to controller args

    // start phase A checks
    if (!check_stack_overflow(controllerArgs->flights)
        || !check_total_size(controllerArgs->flights) ||
        !check_total_keysum(controllerArgs->flights)) {
        pthread_exit((void *) -1);
    }

    // --- all checks passed for phase 1 ---

    printf("\n---------- Phase Switch ----------\n\n");

    // signal to companies to start phase 2
    pthread_barrier_wait(&barrier_start_2nd_phase);
    // wait for companies to finish processing reservations before starting phase 2 checks
    pthread_barrier_wait(&barrier_start_2nd_phase_checks);
    // repeat phase A checks and phase B check
    // for the total size check we must subtract the number of reservations currently in the management center
    if (!check_stack_overflow(controllerArgs->flights)
        || !check_total_size(controllerArgs->flights) ||
        !check_total_keysum(controllerArgs->flights)
        || !reservations_completion_check(controllerArgs->flights, controllerArgs->management_center)) {
        pthread_exit((void *) -1);
    }

    // --- all checks passed for phase 2 ---

    free(controllerArgs);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) exit(-1);

    // read a physical number and convert to int
    int A = atoi(argv[1]);

    // declare the airline companies, flights and agency threads and set global vars
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

    // create reservation management center
    struct list *management_center = create_list();

    for (int i = 0; i < A * A; i++) {
        if (i < A) {
            // init flight reservations table
            flights[i] = (struct flight_reservations *) malloc(sizeof(struct flight_reservations));
            // stack capacity depends on the flight's position in the table
            unsigned int capacity = 1.5f * (A * A) - (A - 1 - i) * A;
            flights[i]->completed_reservations = createStack(capacity);
            flights[i]->pending_reservations = createQueue();

            // init airline companies
            struct airline_args *airline_comp_args = (struct airline_args *) malloc(sizeof(struct airline_args));
            airline_comp_args->flight = flights[i];
            airline_comp_args->management_center = management_center;
            pthread_create(&(airlineCompanies[i]), NULL, airline_main, airline_comp_args);
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
    controllerArgs->management_center = management_center;
    pthread_create(&flight_controller, NULL, flight_controller_main, controllerArgs);

    // wait for agencies, airlines and controller threads to finish
    for (unsigned int i = 0; i < numOfAgencies; i++) {
        pthread_join(agencies[i], NULL);
    }
    for (unsigned int i = 0; i < numOfAirlineCompanies; i++) {
        pthread_join(airlineCompanies[i], NULL);
    }
    pthread_join(flight_controller, NULL);

    // ---------- Memory de-allocation & cleanup ----------

    // destroy barriers and mutexes
    pthread_barrier_destroy(&barrier_start_1st_phase_checks);
    pthread_barrier_destroy(&barrier_start_2nd_phase);
    pthread_barrier_destroy(&barrier_start_2nd_phase_checks);
    pthread_mutex_destroy(&inserter_airlines_lock);

    // free memory for flight stacks, queues and the flight itself
    for (int i = 0; i < numOfFlights; i++) {
        destroyStack(flights[i]->completed_reservations);
        destroyQueue(flights[i]->pending_reservations);
        free(flights[i]);
    }
    destroyList(management_center);
}