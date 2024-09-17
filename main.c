#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"

#define RED "\x1b[31m"
#define RESET "\x1b[0m"

int main(void)
{
    char cwd[1024]; // TODO: Change to max path length
    getcwd(cwd, sizeof(cwd));
    char *prefix = "";
    /* Infinite loop */
    printf("Welcome to the shell\n");
    while (1)
    {
        printf(RED "%s[%s]" RESET "> ", prefix, cwd);

        int buffer = 100;
        char cmd[buffer];
        fgets(cmd, buffer, stdin);
        char *command = strtok(cmd, " ");
        printf("Command: %s\n", command);

        if (strcmp(command, "exit") == 0)
        {
            exit_shell(0);
        }
    }
}