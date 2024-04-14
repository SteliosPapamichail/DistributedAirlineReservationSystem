//
// Created by stelios papamichail csd4020 on 4/7/24.
//

#ifndef HY486_PROJECT_RESERVATIONS_H
#define HY486_PROJECT_RESERVATIONS_H

struct Reservation {
    int agency_id; // the agency_id of the agency that produced this reservation
    int reservation_number;
};

/**
 * Represents a flight's reservations (completed & pending)
 */
struct flight_reservations {
    struct stack *completed_reservations;
    struct queue *pending_reservations;
};


#endif //HY486_PROJECT_RESERVATIONS_H
