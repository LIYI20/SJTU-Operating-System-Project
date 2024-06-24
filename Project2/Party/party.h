#ifndef __PARTY__H__
#define __PARTY__H__

static const int N = 4; // times for dean to enter the room
static const int NS = 50; // number of students

// Don't modify the following lines.

// thread functions
void *dean();
void *student(void *arg);

// functions called in main
void init();
void destroy();

// functions called in threads
void dean_enter();
void breakup();
void search();
void dean_leave();
void student_enter(int id);
void party(int id);
void student_leave(int id);

#endif
