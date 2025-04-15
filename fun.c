#include "fun.h"

int sem_id;
struct sembuf sem_op;
int shm_id;
char *shared_memory;
ChildProcess *children;
int child_count = 0;
int max_children;
int global_counter = 0;
time_t start_time;
volatile sig_atomic_t keep_running = 1;


pid_t find_child(const char *name) {
    for (int i = 0; i < child_count; i++) {
        if (strcmp(children[i].name, name) == 0) {
            return children[i].pid;
        }
    }
    return -1; //If not found
}

void kill_child(const char *name) {
    for (int i = 0; i < child_count; i++) {
        if (strcmp(children[i].name, name) == 0) {
            for (int j = i; j < child_count - 1; j++) {
                children[j] = children[j + 1];
            }
            child_count--;
            return;
        }
    }
}

int check_child(const char *process) {
    time_t start_time = time(NULL);
    time_t end_time;

    signal(SIGUSR1, sigusr_handler);
    signal(SIGTERM, sigterm_handler);

    printf("[Child %d] Process %s started.\n", getpid(), process);
    while (keep_running) {
        pause();
    }

    end_time = time(NULL);
    printf("[Child %d] Terminating. Active time: %ld seconds.\n",
           getpid(), end_time - start_time);
    return 0;
}

void sigusr_handler(int signum) {
    (void)signum;
    for (int i = 0; i < child_count; i++) {
        if (children[i].pid == getpid()) {
            children[i].messages_received++;
            break;
        }
    }
    printf("[Child %d] Woke up, message: \"%s\"\n", getpid(), shared_memory);
    fflush(stdout);
}

void sigterm_handler() {
    keep_running = 0;
}


void semaphore_wait(int sem_index) {
    sem_op.sem_num = sem_index;
    sem_op.sem_op = -1;     // Wait
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void semaphore_signal(int sem_index) {
    sem_op.sem_num = sem_index;
    sem_op.sem_op = 1;      // Signal
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void increment_counter(void) {
    printf("[Parent] Counter = %d\n", global_counter++);
}