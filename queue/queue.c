//
// Created by stelios papamichail csd4020 on 4/7/24.
//

#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// Helper functions to create dummy nodes in order to make working
// with empty and non-empty cases easier
struct queue_reservation *create_dummy_node() {
    struct queue_reservation *node = (struct queue_reservation *) malloc(sizeof(struct queue_reservation));
    if (node == NULL) {
        return NULL;
    }
    node->next = NULL;
    return node;
}

struct queue *createQueue() {
    struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
    if (queue == NULL) {
        return NULL;
    }

    // Create dummy nodes for head and tail
    queue->head = create_dummy_node();
    queue->tail = queue->head;
    queue->size = 0;

    pthread_mutex_init(&(queue->head_lock), NULL);
    pthread_mutex_init(&(queue->tail_lock), NULL);

    return queue;
}

void enqueue(struct queue *queue, struct Reservation reservation) {
    struct queue_reservation *new_node = (struct queue_reservation *) malloc(sizeof(struct queue_reservation));
    if (new_node == NULL) {
        return; //todo: maybe return an error code in order to satisfy totality (slide 2 - lec 5)
    }
    new_node->reservation = reservation;
    new_node->next = NULL;

    // Acquire tail lock first to ensure proper linking
    pthread_mutex_lock(&(queue->tail_lock));

    // Update tail pointer atomically (for concurrent access)
    queue->tail->next = new_node;
    queue->tail = new_node;
    queue->size += 1;
    pthread_mutex_unlock(&(queue->tail_lock));
}

struct Reservation dequeue(struct queue *queue) {
    // Acquire the head lock to ensure proper reading
    pthread_mutex_lock(&(queue->head_lock));

    // Check for empty queue (dummy node check)
    if (queue->head->next == NULL) {
        pthread_mutex_unlock(&(queue->head_lock));
        //todo: maybe return an error code in order to satisfy totality (slide 2 - lec 5)
        return (struct Reservation) {-1, -1};
    }

    // Move head pointer (atomicity not crucial here)
    struct queue_reservation *temp = queue->head->next;
    struct Reservation reservation = temp->reservation;
    queue->head->next = temp->next;
    free(temp);
    queue->size -= 1;
    pthread_mutex_unlock(&(queue->head_lock));

    return reservation;
}

void destroyQueue(struct queue *queue) {
    struct queue_reservation *node = queue->head->next;

    while (node != NULL) {
        struct queue_reservation *temp = node;
        node = node->next;
        free(temp);
    }

    free(queue->head);
    pthread_mutex_destroy(&queue->tail_lock);
    pthread_mutex_destroy(&queue->head_lock);
    free(queue);
}