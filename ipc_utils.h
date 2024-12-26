#ifndef IPC_UTILS_H
#define IPC_UTILS_H

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHARED_MEMORY_NAME "/boat_simulation"
#define BOAT_SEM_NAME "/boat_sem"
#define SEAT_SEM_NAME "/seat_sem"
#define BOARDING_MUTEX_NAME "/boarding_mutex"

typedef struct {
    int remaining_seats;
    int total_passengers;
    int boarded_passengers;
} SharedMemory;

#endif // IPC_UTILS_H
