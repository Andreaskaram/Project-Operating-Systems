#ifndef IPC_UTILS_H
#define IPC_UTILS_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Δημιουργία και αρχικοποίηση σημαφόρων
void init_semaphore(sem_t **sem, const char *name, int value);
void destroy_semaphore(sem_t *sem, const char *name);

// Βοηθητική συνάρτηση για ασφαλή έξοδο από το πρόγραμμα
void handle_error(const char *msg);

#endif // IPC_UTILS_H