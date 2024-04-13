//
// Created by stelios papamichail csd4020 on 4/10/24.
//

#include <malloc.h>
#include "lazy_list.h"


struct list *create_list() {
    struct list *list = (struct list *) malloc(sizeof(struct list));
    list->head = (struct list_reservation *) malloc(sizeof(struct list_reservation));
    list->head->marked = 0;
    list->head->reservation.reservation_number = -1;
    pthread_mutex_init(&list->head->lock, NULL);
    list->tail = (struct list_reservation *) malloc(sizeof(struct list_reservation));;
    list->tail->marked = 0;
    list->tail->reservation.reservation_number = -1;
    pthread_mutex_init(&list->tail->lock, NULL);
    list->head->next = list->tail;
    list->size = 0;
    return list;
}

/**
 *
 * @param pred
 * @param curr
 * @return 1 if valid, 0 if invalid
 */
int validate(struct list_reservation *pred, struct list_reservation *curr) {
    return !pred->marked && !curr->marked && pred->next == curr;
}

int searchReservation(struct list *list, int reservation_number) {

    struct list_reservation *current = list->head;

    while (current != NULL) {
        pthread_mutex_lock(&current->lock);
        if (current->reservation.reservation_number == reservation_number) {
            pthread_mutex_unlock(&current->lock);
            return 1;
        }
        pthread_mutex_unlock(&current->lock);
        current = current->next;
    }

    return 0;
}

void printList(struct list *list) {
    pthread_mutex_lock(&list->head->lock);
    struct list_reservation *current = list->head->next; // head is sentinel
    while (current != list->tail) {
        printf("Reservation in center with id : %d -> ", current->reservation.reservation_number);
        current = current->next;
    }
    pthread_mutex_unlock(&list->head->lock);
    printf("NULL\n");
}

int isListEmpty(struct list *list) {
    return list->size == 0;
}


int insert(struct list *list, struct Reservation reservation) {
    while (1) {
        struct list_reservation *pred = list->head;
        struct list_reservation *curr = list->head->next;

        // find potential suitable position
        while (curr != list->tail) {
            if (curr->reservation.reservation_number >= reservation.reservation_number) break; // position found
            pred = curr;
            curr = curr->next;
        }

        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);

        // confirm that the proper nodes have been locked
        if (validate(pred, curr)) {
            if (curr != list->tail && curr->reservation.reservation_number == reservation.reservation_number) {
                // key already present so abort insertion
                pthread_mutex_unlock(&curr->lock);
                pthread_mutex_unlock(&pred->lock);
                return 0;
            } else {
                // found suitable position for non-yet existent entry
                struct list_reservation *node = (struct list_reservation *) malloc(sizeof(struct list_reservation));
                pthread_mutex_init(&node->lock, NULL);
                node->reservation = reservation;
                node->marked = 0;
                node->next = curr;
                pred->next = node;
                list->size++;
                pthread_mutex_unlock(&curr->lock);
                pthread_mutex_unlock(&pred->lock);

                return 1;
            }
        }

        // failed to validate, release and retry
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_unlock(&pred->lock);
    }
}

/**
 * Removes the first element (lowest reservation number) in the list.
 * @param list
 * @return The first list element
 */
struct Reservation deleteAndGet(struct list *list) {
    struct Reservation reservation;
    while (1) {
        struct list_reservation *pred = list->head;
        struct list_reservation *curr = list->head->next;

        // list is empty
        if (curr == list->tail) {
            reservation.reservation_number = -1;
            return reservation;
        }

        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);

        if (validate(pred, curr)) {
            struct list_reservation *tmp = curr;
            reservation = tmp->reservation;
            curr->marked = 1; // remove logically
            pred->next = curr->next; // remove physically
            list->size--;
            free(tmp);
            pthread_mutex_unlock(&curr->lock);
            pthread_mutex_unlock(&pred->lock);
            return reservation;
        }

        // failed to validate, release and retry
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_unlock(&pred->lock);
    }
}