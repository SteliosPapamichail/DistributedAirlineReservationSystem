//
// Created by fresh on 4/10/24.
//

#ifndef HY486_PROJECT_LAZY_LIST_H
#define HY486_PROJECT_LAZY_LIST_H

#include <pthread.h>
#include "../common/reservations.h"

/**
 * Lazy Synchronization
• “Postpone the hard work for later”: removing an element of a data structure can
   be split in two phases:
– logically remove the element by marking it
– physically remove the element by unlinking it from the rest of the data structure
 */

struct list_reservation {
    struct Reservation reservation;
    int marked; // mark for lazy deletion
    pthread_mutex_t lock;
    struct list_reservation *next;
};

/**
 * A lazy synchronized linked list sorted based on the flight number
 * in ascending order.
 */
struct list {
    struct list_reservation *head;
    struct list_reservation *tail;
    int size;
};

struct list *create_list();

void printList(struct list* list);

int searchReservation(struct list* list, int flight_number);

int validate(struct list_reservation *pred, struct list_reservation *curr);

int isListEmpty(struct list *list);

int insert(struct list *list, struct Reservation reservation);

struct Reservation removeHead(struct list *list);

#endif //HY486_PROJECT_LAZY_LIST_H
