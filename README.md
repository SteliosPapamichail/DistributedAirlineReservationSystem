# Distributed system for producing and managing flight reservations

## System Overview

In the image below, the top `A^2` circles represent the agency threads. Agencies are responsible for producing flight reservations.

In the middle section, we see the flights table containing `A` flights in total, each with its own pending and completed reservations.
Each flight's completed reservations, are stored in a **shared stack using coarsed-grained synchronization**, while its pending
reservations are stored in an **unbounded total queue with sentinel locks**.

Lastly, in the bottom part of the image, we see `A` airline companies represented by threads, which are responsible for managing their
assigned flights and communicating with the reservations management center that is seen at the bottom. The communication between the different
parts of the system is described in the next section.

![image](https://github.com/SteliosPapamichail/DistributedAirlineReservationSystem/assets/58370258/77f80b63-65fc-410e-a956-59367e54aff0)

## Implementation

There are two families of threads (threads), the *agencies*, which perform the production of the flight reservations, and the
*airline companies*, which perform the management of flight reservations. Also, there is an additional thread, the *controller*, which performs some checks to ensure the 
correctness of the system. More specifically, the program accepts an integer A as an argument to the main function upon execution. The number of airlines will be `A` and the multitude of
of agencies will be `P = A2`. Each airline has a unique identifier, which takes a fixed value from 1 to A. Each airline operates only one flight, so the number of available 
flights is A. There is an array named *flights*, of size A, which represents the available flights that can be booked. Each table position in the flights array contains a pointer to a struct, the
`flight_reservations`, which contains pointers to the structures that store the flight reservations. For each position i of the flights table, the reservations located at position i belong to the airline with identifier i+1.

```C
struct flight_reservations {
  struct stack *completed_reservations; 
  struct queue *pending_reservations;
}
```

In the above struct, the field `completed_reservations` is a pointer to a shared stack containing the flight reservations that have been completed, and the field `pending_reservations` is a pointer to a shared queue that contains the pending flight 
reservations. Each stack has a limited capacity. However, it is implemented as a dynamic stack. When a flight is full, additional reservations related to that flight are placed in the flight queue.

```C
struct stack {
  struct stack_reservation* top?
  pthread_mutex_t top_lock; 
  int size;
  int capacity;
}
```

Each node of the stack is represented by the struct:

```C
struct stack_reservation {
  struct Reservation reservation; 
  struct stack_reservation *next;
}
```

The capacity is the stack capacity, i.e. the maximum number of reservations they can to be stored on this stack. The size is the size of the stack, i.e. the number of reservations stored on the stack. The capacity of each stack depends on its position in the flights table. 
Specifically, the stack capacity of the latter table position (i.e. position A-1) should be equal to (3/2)*A2, and the stack of each previous position of the array should have a capacity of A elements less than the stack of the next position. Therefore:

• The stack of the penultimate position of the array (i.e. position A-2) will have a capacity of (3/2)*A2– A
• The stack of the previous position (i.e. position A-3) will have capacity(3/2)*A2– 2*A
• . . .
• The first position stack (ie position0) will have a capacity of (3/2)*A2– (A–1)*A.

The different stack sizes correspond to planes with different numbers of seats. Each reservation, regardless of whether it will be stored on the stack or the queue, is represented by the struct:

```C
struct Reservation {
  int agency_id;
  int reservation_number;
}
```

In the above struct, the `agency_id` is the identifier of the agency that produced it reservation. Each agency has a unique identifier, which takes a fixed value from 1 to P. To `reservation_number` is the reservation number.
When a flight is full, additional reservations related to that flight are placed in the flight queue.

```C
struct queue {
  struct queue_reservation *head;
  struct queue_reservation *tail;
  pthread_mutex_t head_lock; 
  pthread_mutex_t tail_lock;
}
```

Each node of the queue is represented by the struct:

```C
struct queue_reservation {
  struct Reservationreservation; 
  struct queue_reservation *next;
}
```

The agency with identifier `agency_id` produces a total of A bookings with the following numbers reservation:
`i*P + agency_id` where Π is the number of agencies, while i takes the values 0 ≤ i ≤ A–1. Therefore:

• The agency withagency_id=1produces reservations (struct Reservation) with numbers:
1, Π+1, 2*Π+1, 3*Π+1, ..., (A-1)*Π + 1.
• The agency withagency_id=2produces bookings with numbers:
2, Π+2, 2*Π+2, 3*Π+2, ..., (A-1)*Π + 2.
• . . .
• The agency withagency_id=Pproduces bookings with numbers: P, 
2*P, 3*P, 4*P, ..., A*P.

Therefore, the total number of reservations that will be made in the entire system is `A^3`.  The reservation output and stack capacities have been chosen so that:
1) The total capacity of all stacks is sufficient for all bookings.
2) Some stacks (about half) are filled and the remaining reservations for the same 
flight are entered in the flight queue, while in the rest the number of items entered 
is not enough to fill them (so there are queues that will remain empty). These 
stacks will eventually serve the reservations stored in the queues.

## Execution Flow

The implementation of the shared flight reservation system is divided into two phases. The first phase is the phase of creating and inserting the reservations into the stacks and queues 
of the flights table. The second phase is the phase of managing the reservations by the airlines.

## Compilation

Simply run `make` or `make all`.

You can also use `make clean` to delete the generated files.

## Execution

You can run the program by executing the generated executable like so:

`./bin/main A` where `A` is the number of flights.

### By Stelios Papamichail csd4020
