//
// Created by stelios papamichail csd4020 on 4/7/24.
//

#ifndef HY486_PROJECT_STACK_H
#define HY486_PROJECT_STACK_H

#include <pthread.h>
#include <stdbool.h>
#include "../common/reservations.h"

/**
 * A flight reservations
 */
struct stack_reservation {
    struct Reservation reservation;
    struct stack_reservation *next;
};

/**
 * @brief A coarsed-grained lock-based stack for storing flight reservations
 *
 */
struct stack {
    struct stack_reservation *top;
    pthread_mutex_t top_lock;
    unsigned int size; // number of reservations currently stored in the stack
    unsigned int capacity; // maximum number of reservations that can be stored in the stack
};

struct stack *createStack(unsigned int capacity);

bool isStackFull(struct stack *stack);

bool hasStackOverflowed(struct stack *stack);

void push(struct stack *stack, struct Reservation reservation);

struct Reservation pop(struct stack *stack);

void destroyStack(struct stack *stack);

#endif //HY486_PROJECT_STACK_H
