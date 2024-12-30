#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "ipc_utils.h"

#define MAX_PASSENGERS 100

void *passenger(void *arg);  

sem_t *boat_semaphore;  
sem_t *boat_queue;      
sem_t *boat_ready;      

int num_passengers, num_boats, boat_capacity;
int remaining_passengers;
pthread_mutex_t lock;     //Mutex to protect shared variable

void *boat(void *arg) {
    int *boat_id = (int *)arg;

    while (remaining_passengers>0) {
        pthread_mutex_lock(&lock);
        if (remaining_passengers <= 0) {  //Check if there are remaining passengers
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);

        printf("Boat %d is ready to board passengers.\n", *boat_id);


        pthread_mutex_lock(&lock);
        int passengers_to_board = remaining_passengers >= boat_capacity ? boat_capacity : remaining_passengers;
        remaining_passengers -= passengers_to_board;  //remaining passengers after the boat
        pthread_mutex_unlock(&lock);

        //release seats
        for (int i = 0; i < passengers_to_board; i++) {
            sem_post(boat_semaphore);
        }

        for (int i = 0; i < passengers_to_board; i++) {
            sem_post(boat_queue);  // Allow one passenger from the queue
        }

        // Wait for passengers to signal that they are ready
        for (int i = 0; i < passengers_to_board; i++) {
            sem_wait(boat_ready);
        }

        printf("--Boat %d is departing with %d passengers.\n", *boat_id, passengers_to_board);
        //sleep(2);  //Simulate trip
        printf("Boat %d has returned.\n", *boat_id);

        //printf("----%d passengers remaining!\n", remaining_passengers);
    }

    free(boat_id);
    return NULL;
}

int main() {
    printf("Enter the number of passengers: ");
    scanf("%d", &num_passengers);
    printf("Enter the number of boats: ");
    scanf("%d", &num_boats);
    printf("Enter the capacity of each boat: ");
    scanf("%d", &boat_capacity);
    printf("\n");

    if (num_passengers > MAX_PASSENGERS) {
        printf("Error: Maximum number of passengers is %d.\n", MAX_PASSENGERS);
        return EXIT_FAILURE;
    }

    remaining_passengers = num_passengers;
    pthread_mutex_init(&lock, NULL);

    pthread_t passenger_threads[num_passengers];
    pthread_t boat_threads[num_boats];

    //Initialize semaphores
    init_semaphore(&boat_semaphore, "boatsem", 0);
    init_semaphore(&boat_queue, "boatqueue", num_passengers);
    init_semaphore(&boat_ready, "boatready", 0);

    //create boat threads
    for (int i = 0; i < num_boats; i++) {
        int *boat_id = malloc(sizeof(int));
        *boat_id = i + 1;
        pthread_create(&boat_threads[i], NULL, boat, boat_id);
    }

    //create passenger threads
    for (int i = 0; i < num_passengers; i++) {
        int *passenger_id = malloc(sizeof(int));
        *passenger_id = i + 1;
        pthread_create(&passenger_threads[i], NULL, passenger, passenger_id);
    }

    //join passenger threads
    for (int i = 0; i < num_passengers; i++) {
        pthread_join(passenger_threads[i], NULL);
    }

    //join boat threads
    for (int i = 0; i < num_boats; i++) {
        pthread_join(boat_threads[i], NULL);
    }

    //Destroy semaphores
    destroy_semaphore(boat_semaphore, "boatsem");
    destroy_semaphore(boat_queue, "boatqueue");
    destroy_semaphore(boat_ready, "boatready");

    pthread_mutex_destroy(&lock);

    printf("All passengers have been transported. Program exiting.\n");
    return 0;
}