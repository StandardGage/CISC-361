#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "builtin.h"

#define MAX_PATH_LENGTH 1024

int run_builtin(char **args, int argIndex, char **envp)
{
    char *ptr;
    char *noecho = getenv("NOECHO");

    if (strcmp(args[0], "exit") == 0) // exit command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in exit\n");
        }
        int status = argIndex > 1 ? atoi(args[1]) : 0;
        printf("Exiting, status: %d\n", status);
        exit(status);
    }
    else if (strcmp(args[0], "which") == 0) // which command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in which\n");
        }
        if (argIndex < 2)
        {
            fprintf(stderr, "Usage: which [command]\n");
            return 0;
        }
        else
        {
            for (int i = 1; i < argIndex; i++)
            {
                char *result = search_executable(args[i]);
                if (result)
                {
                    printf("%s\n", result);
                    free(result); // Free the result of search_executable
                }
                else
                {
                    printf("%s not found\n", args[i]);
                }
            }
        }
        return 0;
    }
    else if (strcmp(args[0], "list") == 0) // list command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in list\n");
        }
        if (argIndex == 1)
        {
            list_files(".");
        }
        int count = 1;
        while (count < argIndex)
        {
            printf("\n%s:\n", args[count]);
            list_files(args[count]);
            count++;
        }
        return 0;
    }
    else if (strcmp(args[0], "pwd") == 0) // pwd command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in pwd\n");
        }
        ptr = getcwd(NULL, 0); // user might use non-builtin cd
        printf("%s\n", ptr);
        free(ptr);
        return 0;
    }
    else if (strcmp(args[0], "pid") == 0) // pid command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in pid\n");
        }
        printf("PID: %d\n", getpid());
        return 0;
    }
    else if (strcmp(args[0], "printenv") == 0) // printenv command
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in printenv\n");
        }
        if (argIndex == 1)
        {
            print_env(envp);
        }
        else
        {
            int count = 1;
            while (count < argIndex)
            {
                printf("%s=%s\n", args[count], getenv(args[count]));
                count++;
            }
        }
        return 0;
    }
    else if (strcmp(args[0], "setenv") == 0)
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in setenv\n");
        }
        if (argIndex == 1)
        {
            // print whole environment
            print_env(envp);
        }
        else if (argIndex == 2)
        {
            setenv(args[1], "", 1);
        }
        else if (argIndex == 3)
        {
            setenv(args[1], args[2], 1);
        }
        else
        {
            // print to stderr
            fprintf(stderr, "Usage: setenv [var] [value]\n");
        }
        return 0;
    }
    else if (strcmp(args[0], "addacc") == 0)
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in addacc\n");
        }
        char *acc_value_str = getenv("ACC");
        int acc_value = 0;
        if (acc_value_str != NULL)
        {
            acc_value = atoi(acc_value_str);
        }

        int add_value = 1;
        if (argIndex > 1)
        {
            add_value = atoi(args[1]);
        }

        int new_acc_value = acc_value + add_value;

        char new_acc_str[20];
        sprintf(new_acc_str, "%d", new_acc_value);

        setenv("ACC", new_acc_str, 1);

        return 0;
    }
    else
    {
        return -1;
    }
}

void list_files(char *dir) // list files in a directory
{
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir);
    if (dp != NULL)
    {
        while ((ep = readdir(dp)))
        {
            printf("%s\n", ep->d_name);
        }
        (void)closedir(dp);
    }
    else
    {
        perror("Couldn't open the directory");
    }
}

void print_env(char **envp) // print environment variables
{
    if (envp != NULL)
    {
        for (char **current = envp; *current != NULL; current++)
        {
            // Split the string at the '=' character
            char *key = strdup(*current);
            char *value = strchr(key, '=');
            if (value != NULL)
            {
                *value = '\0'; // Null-terminate the key
                value++;       // Move to the value part

                printf("%s=%s\n", key, getenv(key));
            }
            else
            {
                printf("Invalid environment variable format: %s\n", *current);
            }
            free(key);
        }
    }
    else
    {
        printf("No environment variables found\n");
    }
}

char *search_executable(const char *command)
{
    char *path = getenv("PATH");
    if (!path)
    {
        fprintf(stderr, "PATH not set\n");
        return NULL;
    }

    // Make a copy of the PATH because strtok modifies the string
    char *path_copy = strdup(path);
    if (!path_copy)
    {
        perror("strdup");
        return NULL;
    }

    char *directory = strtok(path_copy, ":");
    char full_path[MAX_PATH_LENGTH];

    while (directory != NULL)
    {

        snprintf(full_path, sizeof(full_path), "%s/%s", directory, command);

        // Check if the file exists and is executable
        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);          // Free the copy of the PATH
            return strdup(full_path); // Return a copy of the found executable path
        }

        // Continue searching in the next directory
        directory = strtok(NULL, ":");
    }

    free(path_copy); // Free the copy of the PATH
    return NULL;     // Return NULL if the executable wasn't found
}