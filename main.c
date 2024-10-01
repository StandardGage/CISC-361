#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAXLINE 128
#define MAXARGS 10
#define MAX_PATH_LENGTH 1024

// parse command entered
int main()
{

    char buf[MAXLINE];
    pid_t pid;
    int status;
    char *ptr;
    int background;
    char *cwd = getcwd(NULL, 0);
    char *args[MAXARGS];
    char *token;

    char prefix[MAXLINE] = "";

    background = 1;

    printf("Welcome to the shell!\n");
    while (1)
    {
        printf("%s [%s]> ", prefix, cwd);

        if (fgets(buf, MAXLINE, stdin) == NULL)
        {
            // Handle EOF
            printf("exit\n");
            break;
        }

        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0; /* replace newline with null */

        if (strlen(buf) == 0)
        {
            continue;
        }
        int argIndex = 0;
        token = strtok(buf, " ");
        while (token != NULL && argIndex < MAXARGS - 1)
        {
            args[argIndex] = token;
            argIndex++;
            token = strtok(NULL, " ");
        }
        args[argIndex] = NULL;

        if (strcmp(args[0], "exit") == 0)
        {
            int status = argIndex > 1 ? atoi(args[1]) : 0;
            printf("Exiting, status: %d\n", status);
            exit(status);
        }
        else if (strcmp(args[0], "which") == 0)
        {
            find_command(args[1]);
        }
        else if (strcmp(args[0], "list") == 0)
        { 
                if (argIndex == 1) {
                    list_dirs(".");
                }
                int count = 1;
                while (count < argIndex) {
                    printf("\n%s:\n", args[count]);
                    list_dirs(args[count]);
                    count++;
                }
            
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            ptr = getcwd(NULL, 0);
            printf("CWD = [%s]\n", ptr);
            free(ptr);
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (argIndex == 1)
            {
                chdir(getenv("HOME"));
            }
            if (strcmp(args[1], "-") == 0)
            {
                chdir(strcat(cwd,"/.."));
            }
            else
            {
                chdir(args[1]);
            }
            cwd = getcwd(NULL, 0);
        }
        else if (strcmp(args[0], "pid") == 0)
        {
            printf("PID: %d\n", getpid());
        }
        else if (strcmp(args[0], "prompt") == 0)
        {
            if (argIndex == 2)
            {
                strcpy(prefix, args[1]);
                if (prefix[strlen(prefix) - 1] == '\n')
                    prefix[strlen(prefix) - 1] = 0; /* replace newline with null */
            }
            else
            {
                printf("Enter new prompt: ");
                fgets(prefix, MAXLINE, stdin);
                if (prefix[strlen(prefix) - 1] == '\n')
                    prefix[strlen(prefix) - 1] = 0; /* replace newline with null */
            }
        }
        else
        {
            if ((pid = fork()) < 0)
            {
                printf("fork error\n");
                exit(1);
            }
            else if (pid == 0)
            {
                execlp(buf, buf, (char *)0);
                printf("couldn't execute: %s\n", buf);
                exit(127);
            }

            if (!background)
            {
                if ((pid = waitpid(pid, &status, 0)) < 0)
                    printf("waitpid error\n");
            }
            else
            {
                // save pid somewhere for later
            }
        }
        pid = waitpid(pid, &status, WNOHANG);
    }
    exit(0);
}

int is_executable(char *directory, char *command)
{
    char full_path[MAX_PATH_LENGTH];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, command);

    struct stat buffer;
    if (stat(full_path, &buffer) == 0 && buffer.st_mode & S_IXUSR)
    {
        printf("%s\n", full_path);
        return 1;
    }
    return 0;
}

void find_command(char *command)
{
    char *p = getenv("PATH");
    if (!p)
    {
        fprintf(stderr, "PATH not set\n");
        return;
    }

    char *path = strdup(p);
    if (!path)
    {
        perror("strdup");
        return;
    }

    char *directory = strtok(path, ":");
    while (directory != NULL)
    {
        if (is_executable(directory, command))
        {
            return;
        }
        directory = strtok(NULL, ":");
    }

    printf("%s: Command not found\n", command);
}

void list_dirs(char *directory)
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir(directory);

    while ((entry = readdir(dir)) != NULL)
    {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}