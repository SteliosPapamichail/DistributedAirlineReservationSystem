//
// Created by fresh on 4/10/24.
//

#include <malloc.h>
#include "lazy_list.h"

struct list *create_list() {
    struct list *list = (struct list *) malloc(sizeof(struct list));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/**
 *
 * @param pred
 * @param curr
 * @return 1 if valid, 0 if invalid
 */
int validate(struct list_reservation *pred, struct list_reservation *curr) {
    if (!pred->marked && !curr->marked && pred->next == curr) {
        return 1;
    } else {
        return 0;
    }
}

/**
 *
 * @param reservation_number
 * @return  1 if found and not marked, 0 otherwise
 */
int search(struct list *list, int reservation_number) {
    struct list_reservation *curr = list->head;

    while (curr->reservation.reservation_number < reservation_number) {
        curr = curr->next;
    }

    if (!curr->marked && curr->reservation.reservation_number == reservation_number) {
        return 1;
    } else {
        return 0;
    }
}

/**
 *
 * @param list
 * @param reservation
 * @return 1 on success, 0 otherwise
 */
int insert(struct list *list, struct Reservation reservation) {
    struct list_reservation *pred, *curr;
    int result;
    int return_flag = 0;

    while (1) {
        pred = list->head;
        curr = pred->next;

        // traverse until a suitable position
        while (curr->reservation.reservation_number < reservation.reservation_number) {
            pred = curr;
            curr = pred->next;
        }

        // lock both nodes for potential insertion
        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);

        if (validate(pred, curr)) {
            // flight with this number already exists, abort
            if (reservation.reservation_number == curr->reservation.reservation_number) {
                result = 0;
                return_flag = 1;
            } else {
                // proceed with insertion
                struct list_reservation *node = (struct list_reservation *) malloc(sizeof(struct list_reservation));
                node->next = curr;
                pthread_mutex_init(&node->lock, NULL);
                node->reservation = reservation;
                pred->next = node;
                result = 1;
                return_flag = 1;
            }
        }

        // release locks
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_unlock(&pred->lock);

        if (return_flag) return result;
    }
}

int delete(struct list *list, int reservation_number) {
    struct list_reservation *pred, *curr;
    int result;
    int return_flag = 0;

    // similar logic to insert for the most part
    while (1) {
        pred = list->head;
        curr = pred->next;
        while (curr->reservation.reservation_number < reservation_number) {
            pred = curr;
            curr = curr->next;
        }

        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);

        if (validate(pred, curr)) {
            if (curr->reservation.reservation_number == reservation_number) {
                curr->marked = 1; // mark for lazy deletion
                pred->next = curr->next; // unlink from the list
                result = 1;
            } else {
                result = 0;
            }
            return_flag = 1;
        }
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_unlock(&pred->lock);

        if (return_flag) return result;
    }
}