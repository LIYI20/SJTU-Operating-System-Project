#include "party.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *dean() {
    for (int i = 0; i < N; i++) {
        usleep((rand() % 500 + 200) * 1000);
        // some code goes here
    }
    pthread_exit(0);
}

void *student(void *arg) {
    int id = *(int *)arg;
    // some code goes here
    pthread_exit(0);
}

void init() {
    // some code goes here
}

void destroy() {
    // some code goes here
}
