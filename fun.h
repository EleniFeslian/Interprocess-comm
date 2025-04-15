#ifndef FUN_H
#define FUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>

#define SHM_SIZE 256

typedef struct {
    char name[10];
    pid_t pid;
    int messages_received;
    time_t start_time;
} ChildProcess;

extern int sem_id;
extern struct sembuf sem_op;
extern int shm_id;
extern char *shared_memory;
extern ChildProcess *children;
extern int child_count;
extern int max_children;
extern int global_counter;
extern time_t start_time;
extern volatile sig_atomic_t keep_running;

pid_t find_child(const char *name);
void kill_child(const char *name);
int check_child(const char *process);
void sigusr_handler(int signum);
void sigterm_handler();
void semaphore_wait(int sem_index);
void semaphore_signal(int sem_index);
void increment_counter(void);

#endif