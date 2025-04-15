# Interprocess Communication

## Project Structure

This project is organized into three main files:

- **`fun.h`**: A header file that contains function declarations, as well as any constants and variables that need to be accessible across all source files.
- **`fun.c`**: Implements the functions declared in `fun.h`.
- **`main.c`**: Contains the `main` function and handles the core functionality of the project based on the assignment requirements.

A **`Makefile`** is also included to simplify the compilation and execution process.


This program simulates a process (P) that reads and executes instructions from a command file (CF), which follows a specific structure:

- **Timestamp** – **Process** – **Command**
  
  The **Timestamp** refers to an integer "timestamp" that indicates when a specific action should occur during the execution of the program. The **Process** refers to the label of the process (e.g., C3), and the **Command** can take one of two values:
  - **S (SPAWN)**: This command instructs the creation of a new child process from process P.
  - **T (TERMINATE)**: This command signals the termination of the specified process.

Process P also has access to an external text file, which is passed as a parameter upon its initialization. After starting the processes, in each cycle of operation, process P randomly selects one of the active child processes and sends it a random line of text from the external file. The receiving process prints the line to the standard output.

### Process Synchronization

To manage communication between process P and its child processes (Ci), a semaphore table of maximum size **M** is created, which limits the number of active child processes at any given time. This size is also specified as a parameter during the initialization of process P. When a new child process is spawned, it is assigned a semaphore from this table to facilitate communication with its parent, P.

When a child process terminates (via a T command), the corresponding semaphore can be reused for the creation of new child processes. 

### Shared Memory

For data exchange between process P and its child processes, a shared memory segment is utilized. This shared memory allows the communication of text lines from P to its children and ensures synchronization across processes.
