#include "lcm.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// semaphores
// sem_t shovel;

void *larry() {
    // some code goes here
    /******************************
    int id = 0;
    while (1) {
        sem_wait(&shovel);
        get_shovel(LARRY);

        dig(LARRY, ++id);
        drop_shovel(LARRY);
        sem_post(&shovel);

        pthread_exit(0);
    }
    ******************************/
}

void *moe() {
    // some code goes here
    /******************************
    int id = 0;
    while (1) {
        plant(MOE, ++id);
        pthread_exit(0);
    }
    ******************************/
}

void *curly() {
    // some code goes here
    /******************************
    int id = 0;
    while (1) {
        get_shovel(CURLY);
        fill(CURLY, ++id);

        drop_shovel(CURLY);
        pthread_exit(0);
    }
    ******************************/
}

void init() {
    // some code goes here
    // sem_init(&shovel, 0, 1);
}

void destroy() {
    // some code goes here
    // sem_destroy(&shovel);
}
