#include "ipc_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

void *launch_thread(void *arg);

typedef struct {
    int launch_id;
    int seats_per_launch;
    sem_t *boat_sem;
    sem_t *seat_sem;
    sem_t *boarding_mutex;
    SharedMemory *shm;
} LaunchArgs;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <total_passengers> <num_launches> <seats_per_launch>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int total_passengers = atoi(argv[1]);
    int num_launches = atoi(argv[2]);
    int seats_per_launch = atoi(argv[3]);

    if (total_passengers <= 0 || num_launches <= 0 || seats_per_launch <= 0) {
        fprintf(stderr, "All inputs must be positive integers.\n");
        exit(EXIT_FAILURE);
    }

    // Create shared memory
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    shm->remaining_seats = seats_per_launch;
    shm->total_passengers = total_passengers;
    shm->boarded_passengers = 0;

    // Create semaphores
    sem_t *boat_sem = sem_open(BOAT_SEM_NAME, O_CREAT, 0666, num_launches);
    sem_t *seat_sem = sem_open(SEAT_SEM_NAME, O_CREAT, 0666, seats_per_launch);
    sem_t *boarding_mutex = sem_open(BOARDING_MUTEX_NAME, O_CREAT, 0666, 1);

    if (boat_sem == SEM_FAILED || seat_sem == SEM_FAILED || boarding_mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    pthread_t launch_threads[num_launches];
    LaunchArgs launch_args[num_launches];

    // Create threads for launches
    for (int i = 0; i < num_launches; i++) {
        launch_args[i].launch_id = i + 1;
        launch_args[i].boat_sem = boat_sem;
        launch_args[i].seat_sem = seat_sem;
        launch_args[i].boarding_mutex = boarding_mutex;
        launch_args[i].shm = shm;

        pthread_create(&launch_threads[i], NULL, launch_thread, &launch_args[i]);
    }

    // Wait for all launches to finish
    for (int i = 0; i < num_launches; i++) {
        pthread_join(launch_threads[i], NULL);
    }

    printf("Simulation ended.\n");

    // Cleanup resources
    sem_close(boat_sem);
    sem_close(seat_sem);
    sem_close(boarding_mutex);
    sem_unlink(BOAT_SEM_NAME);
    sem_unlink(SEAT_SEM_NAME);
    sem_unlink(BOARDING_MUTEX_NAME);
    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}

void *launch_thread(void *arg) {
    LaunchArgs *args = (LaunchArgs *)arg;

    while (1) {
        sem_wait(args->boat_sem);

        sem_wait(args->boarding_mutex);
        if (args->shm->boarded_passengers >= args->shm->total_passengers) {
            sem_post(args->boarding_mutex);
            sem_post(args->boat_sem);
            break;
        }

        printf("Launch %d departing...\n", args->launch_id);

        sleep(2); // Simulate trip

        args->shm->remaining_seats = args->shm->remaining_seats + args->shm->total_passengers;
        printf("Launch %d returned to dock.\n", args->launch_id);

        sem_post(args->seat_sem);
        sem_post(args->boarding_mutex);
        sem_post(args->boat_sem);
    }

    return NULL;
}
