#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "ipc_utils.h"

extern sem_t *boat_semaphore;     // Έλεγχος για τις διαθέσιμες θέσεις στη λέμβο
extern sem_t *boat_queue;        // Ουρά για επιβάτες που περιμένουν να επιβιβαστούν
extern sem_t *boat_ready;        // Σηματοδότηση ότι η λέμβος είναι έτοιμη να αναχωρήσει

void *passenger(void *arg) {
    int passenger_id = *(int *)arg;

    printf("Passenger %d is waiting to board.\n", passenger_id);

    //sem_wait(boat_queue);      // Wait for the boat to signal availability
    sem_wait(boat_semaphore);  // Attempt to board

    printf("Passenger %d boarded the boat.\n", passenger_id);

    sem_post(boat_ready);      // Signal the boat that a passenger has boarded

    free(arg);
    return NULL;
}