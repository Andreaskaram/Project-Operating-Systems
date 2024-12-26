#include "ipc_utils.h" // Include the header file for SharedMemory and semaphore definitions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

int main() {
    // Open semaphores
    sem_t *boat_sem = sem_open(BOAT_SEM_NAME, 0);
    sem_t *seat_sem = sem_open(SEAT_SEM_NAME, 0);
    sem_t *boarding_mutex = sem_open(BOARDING_MUTEX_NAME, 0);

    if (boat_sem == SEM_FAILED || seat_sem == SEM_FAILED || boarding_mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    // Open shared memory
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Wait for a seat to become available
        sem_wait(boat_sem);
        sem_wait(boarding_mutex);

        if (shm->boarded_passengers >= shm->total_passengers) {
            sem_post(boarding_mutex);
            sem_post(boat_sem);
            break;
        }

        if (shm->remaining_seats > 0) {
            shm->remaining_seats--;
            shm->boarded_passengers++;
            printf("Passenger boarded. Remaining seats: %d\n", shm->remaining_seats);
            sem_post(boarding_mutex);
            sem_post(seat_sem);
        } else {
            sem_post(boarding_mutex);
        }
    }

    // Cleanup
    sem_close(boat_sem);
    sem_close(seat_sem);
    sem_close(boarding_mutex);
    munmap(shm, sizeof(SharedMemory));
    close(shm_fd);

    return 0;
}
