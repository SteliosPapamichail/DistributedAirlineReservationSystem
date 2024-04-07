//
// Created by fresh on 4/7/24.
//

#ifndef HY486_PROJECT_QUEUE_H
#define HY486_PROJECT_QUEUE_H

#include "../common/reservations.h"
#include <pthread.h>

struct queue_reservation {
    struct Reservation reservation;
    struct queue_reservation *next;
};

/**
 * @brief An unbounded total queue that uses locks for
 * the head and tail.
 */
struct queue {
    struct queue_reservation *head;
    struct queue_reservation *tail;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;
};

struct queue_reservation* create_dummy_node();

struct queue *createQueue();

void enqueue(struct queue *queue, struct Reservation reservation);

struct Reservation dequeue(struct queue *queue);

void destroyQueue(struct queue *queue);

#endif //HY486_PROJECT_QUEUE_H