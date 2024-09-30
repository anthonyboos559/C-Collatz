#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void init_pipes(int* pipes, int n_pipes) {
    for (int i = 0; i < n_pipes; i++) {
        if (pipe(&pipes[i * 2]) == -1) {
            perror("Failed to instaniate pipes!\n");
            exit(1);
        }
    }
}

void close_pipes(int* data_pipes, int* message_pipes, int n_pipes, int write_idx, int read_idx) {
    for (int i = 0; i < n_pipes; i++) {
        if (i == write_idx) {
            continue;
        } 
        else if (i == read_idx) {
            close(message_pipes[i]);
        }
        else {
            close(data_pipes[i]);
            close(message_pipes[i]);
        }
    }    
}

int main(int argc, char* argv[]) {

    // Ensuring the number of child processes is provided
    if (argc < 2) {
        perror("Please provide a number of child processes to spawn\n");
        return 1;
    }
    
    // Converting and checking the provided int is valid
    // atoi returns 0 on failure so we can check both simultaneously
    int num_children = atoi(argv[1]);
    if (num_children <= 0) {
        perror("Invalid number of child processes requested\n");
        return 1;
    }

    // One pipe for sibling communication and one for parent communication
    int* data_pipes = malloc(num_children * 2 * sizeof(int));
    int* message_pipes = malloc(num_children * 2 * sizeof(int));

    if (data_pipes == NULL || message_pipes == NULL) {
        perror("Failed to allocate pipe memory!\n");
        exit(1);
    }

    init_pipes(data_pipes, num_children);
    init_pipes(message_pipes, num_children);


    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork!\n");
            exit(1);
        } 
        else if (pid == 0) {
            if (i == 0) {
                int data_in = (num_children * 2) - 2;
                int data_out = i + 1;
                close_pipes(data_pipes, message_pipes, num_children, data_out, data_in);
            }
            else {
                int data_in = (i * 2) - 2;
                int data_out = (i * 2) + 1;
                close_pipes(data_pipes, message_pipes, num_children, data_out, data_in);
            }
        }
        else {
            close(message_pipes[(i * 2) + 1]);
            close(data_pipes[i * 2]);
            if (i != (num_children - 1)) {
                close(data_pipes[(i * 2) + 1]);
            }
        }
    }


    return 0;
}