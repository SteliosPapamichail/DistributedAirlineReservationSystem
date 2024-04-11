//
// Created by fresh on 4/10/24.
//

#include <malloc.h>
#include "lazy_list.h"


struct list *create_list() {
    struct list *list = (struct list *) malloc(sizeof(struct list));
    list->head = NULL;
    list->tail = NULL;
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
    if (pred != NULL && curr != NULL && !pred->marked && !curr->marked && pred->next == curr) {
        return 1;
    } else {
        return 0;
    }
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
    struct list_reservation *current = list->head;
    while (current != NULL) {
        pthread_mutex_lock(&current->lock);
        printf("Reservation in center with id : %d -> ", current->reservation.reservation_number);
        pthread_mutex_unlock(&current->lock);
        current = current->next;
    }
    printf("NULL\n");
}

int isListEmpty(struct list *list) {
    return list->size == 0;
}

/**
 *
 * @param list
 * @param reservation
 * @return 1 on success, 0 otherwise
 */
int insert(struct list *list, struct Reservation reservation) {
    struct list_reservation *new_node = (struct list_reservation *) malloc(sizeof(struct list_reservation));
    if (!new_node) return 0;
    new_node->reservation = reservation;
    new_node->marked = 0;
    pthread_mutex_init(&new_node->lock, NULL);
    new_node->next = NULL;

    pthread_mutex_lock(&new_node->lock);

    if (list->head == NULL) { // list is empty
        list->head = new_node;
        list->tail = new_node;
        list->size++;
        pthread_mutex_unlock(&new_node->lock);
        return 1;
    }

    struct list_reservation *current = list->head;
    struct list_reservation *previous = NULL;

    // find sorted position
    while (current != NULL && current->reservation.reservation_number < reservation.reservation_number) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) { // Insert at the beginning
        new_node->next = list->head;
        list->head = new_node;
    } else { // Insert in sorted position
        previous->next = new_node;
        new_node->next = current;
        if (current == NULL)
            list->tail = new_node;
    }

    list->size++;

    pthread_mutex_unlock(&new_node->lock);

    return 1;
}

/**
 * Removes the first element (lowest reservation number) in the list.
 * @param list
 * @return The first list element
 */
struct Reservation removeHead(struct list *list) {
    pthread_mutex_lock(&list->head->lock);
    struct Reservation removed_reservation;

    if (list->head != NULL) {
        struct list_reservation *temp = list->head;
        removed_reservation = temp->reservation;

        list->head = list->head->next;
        free(temp);

        list->size--;

        if (list->head == NULL) {
            list->tail = NULL;
        }
    } else {
        // list was empty
        removed_reservation.reservation_number = -1;
    }

    pthread_mutex_unlock(&list->head->lock);
    return removed_reservation;
}