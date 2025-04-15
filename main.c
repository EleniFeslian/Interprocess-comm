#include "fun.h"

int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf("Usage: %s <config_file> <text_file> <num_semaphores>\n", argv[0]);
        return 1;
    }

    max_children = atoi(argv[3]);
    if (max_children <= 0) {
        printf("[Parent] Invalid number of semaphores specified: %d\n", max_children);
        return 1;
    }

    children = malloc(max_children * sizeof(ChildProcess));
    if (children == NULL) {
        perror("malloc");
        return 1;
    }

    // Shared memory creation + attachment
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        free(children);
        return 1;
    }
    shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        free(children);
        return 1;
    }

    // Semaphore creation
    sem_id = semget(IPC_PRIVATE, max_children, IPC_CREAT | 0666);
    if (sem_id < 0) {
        perror("semget");
        shmdt(shared_memory);
        free(children);
        return 1;
    }

    // Initialize semaphores
    for (int i = 0; i < max_children; i++) {
        semctl(sem_id, i, SETVAL, 0);
    }

    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL) {
        perror("[Parent] Error opening configuration file");
        free(children);
        return 1;
    }

    FILE *text_file = fopen(argv[2], "r");
    if (text_file == NULL) {
        perror("[Parent] Error opening text file");
        fclose(config_file);
        free(children);
        return 1;
    }

    // Read text file
    char **text_lines = NULL;
    int text_lines_count = 0;
    char buffer[SHM_SIZE];
    while (fgets(buffer, sizeof(buffer), text_file)) {
        text_lines = realloc(text_lines, (text_lines_count + 1) * sizeof(char *));
        text_lines[text_lines_count] = strdup(buffer);
        text_lines_count++;
    }
    fclose(text_file);

    // Signal handler for SIGUSR1
    signal(SIGUSR1, sigusr_handler);

    int loop_end = -1;
    fseek(config_file, 0, SEEK_END);
    long file_size = ftell(config_file);

    for (long i = file_size - 1; i >= 0; i--) {
        fseek(config_file, i, SEEK_SET);
        if (fgetc(config_file) == '\n' || i == 0) {
            if (i == 0) fseek(config_file, i, SEEK_SET);
            if (fscanf(config_file, "%d", &loop_end)) {
                break;
            }
        }
    }

    for (int i = 0; i <= loop_end; i++) {
        //printf("Loop %d\n", i);
        increment_counter();
        rewind(config_file);

        int line_number;
        char process_label[10];
        char operation[10];
        int matched = 0;

        while (fscanf(config_file, "%d %s %s", &line_number, process_label, operation) == 3) {
            if (line_number == i) {
                if (strcmp(operation, "S") == 0) {
                    if (child_count < max_children) {
                        pid_t pid = fork();
                        if (pid == 0) {
                            exit(check_child(process_label));
                        } else if (pid > 0) {
                            strcpy(children[child_count].name, process_label);
                            children[child_count].pid = pid;
                            child_count++;
                        } else {
                            perror("fork");
                        }
                    } else {
                        printf("[Parent] Max children reached, cannot create process %s.\n", process_label);
                    }
                } else if (strcmp(operation, "T") == 0) {
                    pid_t pid_to_kill = find_child(process_label);
                    if (pid_to_kill != -1) {
                        kill(pid_to_kill, SIGTERM);
                        printf("[Parent] Child %s with PID %d terminated.\n", process_label, pid_to_kill);
                        kill_child(process_label);
                    } else {
                        printf("[Parent] No child with name %s found.\n", process_label);
                    }
                }
                break;
            }
        }

        if (!matched && child_count > 0 && i != loop_end) {
            int random_child = rand() % child_count;
            int random_line = rand() % text_lines_count;
            strcpy(shared_memory, text_lines[random_line]);
            printf("[Parent] Sent line \"%s\" to child %s (PID: %d).\n",
                   text_lines[random_line], children[random_child].name, children[random_child].pid);
            kill(children[random_child].pid, SIGUSR1);
        }

        if (i == loop_end) {
            printf("[Parent] Reached loopEnd (%d)\n", loop_end);
            break;
        }

        usleep(500000);
    }

    for (int i = 0; i < child_count; i++) {
        kill(children[i].pid, SIGKILL);
        printf("[Parent] Remaining child %s with PID %d terminated.\n", children[i].name, children[i].pid);
    }

    fclose(config_file);

    for (int i = 0; i < text_lines_count; i++) {
        free(text_lines[i]);
    }
    free(text_lines);

    shmdt(shared_memory);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    free(children);

    return 0;
}
