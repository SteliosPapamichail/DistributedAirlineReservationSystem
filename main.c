#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

struct flight_reservations {
    struct stack *completed_reservations;
    struct queue *pending_reservations;
};

int main(int argc, char *argv[]) {
    if(argc > 2) exit(-1);

    // read a physical number ( # of agencies = A^2 and # of airline companies = A), available
    // flights = A
    int A = atoi(argv[1]);


}