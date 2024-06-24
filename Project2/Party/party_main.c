// Don't modify this file.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "party.h"

int main() {
    init();
    printf("N = %d.\nStudents = %d.\n", N, NS);

    pthread_t dean_t;
    pthread_t t[NS];
    int id[NS];

    pthread_create(&dean_t, NULL, dean, NULL);

    for (int i = 0; i < NS; i++) {
        usleep((rand() % 300 + 100) *
               1000);  // sleep for a random time, 100-400 ms
        id[i] = i + 1;
        pthread_create(&t[i], NULL, student, (void *)&id[i]);
    }

    for (int i = 0; i < NS; i++) {
        pthread_join(t[i], NULL);
    }
    pthread_join(dean_t, NULL);

    destroy();
    printf("End.\n");
}

void dean_enter() {
    printf("The Dean of students enters the room.\n");
    usleep((rand() % 200 + 200) * 1000);
}

void breakup() {
    printf("The Dean of students breaks up the party.\n");
    usleep((rand() % 2000 + 1000) * 1000);
}

void search() {
    printf("The Dean of students searches the room.\n");
    usleep((rand() % 2000 + 1000) * 1000);
}

void dean_leave() { printf("The Dean of students leaves the room.\n"); }

void student_enter(int id) {
    usleep((rand() % 200 + 200) * 1000);
    printf("Student #%d enters the room.\n", id);
}

void party(int id) {
    usleep((rand() % 500 + 100) * 1000);
    printf("Student #%d joins the party.\n", id);
    usleep((rand() % 2000 + 1000) * 1000);
}

void student_leave(int id) { printf("Student #%d leaves the room.\n", id); }
