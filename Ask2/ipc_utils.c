#include "ipc_utils.h"
#include <fcntl.h>    // For O_CREAT
#include <sys/stat.h> // For mode_t

// Initialize the semaphore using sem_open
void init_semaphore(sem_t **sem, const char *name, int value) {
    *sem = sem_open(name, O_CREAT, 0666, value);
    if (*sem == SEM_FAILED) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    } else {
        //printf("Semaphore successfully created with value %d.\n", value);
    }
}

// Close and unlink the semaphore (proper cleanup)
void destroy_semaphore(sem_t *sem, const char *name) {
    if (sem == SEM_FAILED) {
        perror("Invalid semaphore, cannot close");
        exit(EXIT_FAILURE);
    }

    // Close the semaphore
    if (sem_close(sem) != 0) {
        perror("Semaphore closing failed");
        exit(EXIT_FAILURE);
    } else {
        //printf("Semaphore closed successfully.\n");
    }

    // Unlink the semaphore
    if (sem_unlink(name) != 0) {
        perror("Semaphore unlinking failed");
        exit(EXIT_FAILURE);
    } else {
        //printf("Semaphore unlinked successfully.\n");
    }
}

// Error handling function
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}