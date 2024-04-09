//
// Created by fresh on 4/7/24.
//

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

struct stack *createStack(unsigned int capacity) {
    struct stack *newStack = (struct stack *) malloc(sizeof(struct stack));
    if (newStack == NULL) {
        return NULL;
    }
    newStack->top = NULL;
    pthread_mutex_init(&(newStack->top_lock), NULL);
    newStack->size = 0;
    newStack->capacity = capacity;
    return newStack;
}

bool isStackFull(struct stack *stack) {
    return stack->size == stack->capacity;
}

bool hasStackOverflowed(struct stack *stack) {
    return stack->size > stack->capacity;
}

void push(struct stack *stack, struct Reservation reservation) {
    if (stack->size == stack->capacity) {
        return;
    }

    // create thew new reservation
    struct stack_reservation *newNode = (struct stack_reservation *) malloc(sizeof(struct stack_reservation));
    if (newNode == NULL) {
        return;
    }
    newNode->reservation = reservation;
    newNode->next = stack->top;

    // Lock the stack before modifying it
    pthread_mutex_lock(&(stack->top_lock));
    stack->top = newNode;
    stack->size += 1;
    pthread_mutex_unlock(&(stack->top_lock));
}

struct Reservation pop(struct stack *stack) {
    if (stack->top == NULL) {
        // Handle empty stack
        printf("Could not retrieve reservation from stack. Stack is empty!");
        return (struct Reservation) {0}; // Or a placeholder for empty reservation
    }

    // Lock the stack before modifying it
    pthread_mutex_lock(&(stack->top_lock));
    struct stack_reservation *temp = stack->top;
    struct Reservation reservation = temp->reservation;
    stack->top = temp->next;
    stack->size -= 1;
    free(temp);
    pthread_mutex_unlock(&(stack->top_lock));

    return reservation;
}

void destroyStack(struct stack *stack) {
    if (stack == NULL) {
        return;
    }

    // Lock the stack before destroying elements
    pthread_mutex_lock(&(stack->top_lock));
    while (stack->top != NULL) {
        struct stack_reservation *temp = stack->top;
        stack->top = temp->next;
        free(temp);
    }
    pthread_mutex_destroy(&(stack->top_lock));
    free(stack);
}