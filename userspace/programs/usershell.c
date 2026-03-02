#include <string.h>
#include <syscalls.h>
#include <printf.h>

void execute_command(char *line) {
    if (!line || line[0] == '\0') return;

    if(strcmp(line, "help") == 0) {
        printf("Available commands: help, about\n");
    }
    else if (strcmp(line, "about") == 0) {
        printf("Estella userspace shell\n");
    }
    else {
        printf("Unknown command: %s \n", line);
    }
}

void main(void) {
    printf("Welcome to Estella userspace shell! \n");
    printf("Type 'help' for commands. \n\n");

    char line[128];

    while(1) {
        printf("sh> ");

        int n = read(0, line, sizeof(line) - 1);

        if (n > 0) {
            line[n] = '\0';
            execute_command(line);
        }
    }
}