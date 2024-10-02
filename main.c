#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXLINE 128
#define MAXARGS 10
#define MAX_PATH_LENGTH 1024

char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);
void initialize_readline();

// parse command entered
int main(int argc, char *argv[], char **envp)
{

    char *input;
    pid_t pid;
    int status;
    char *ptr;
    int background;
    char *cwd = getcwd(NULL, 0);
    char *args[MAXARGS];
    char *token;

    char prefix[MAXLINE] = "";

    background = 1;

    initialize_readline();

    printf("Welcome to the shell!\n");
    while (1)
    {
        char prompt[MAXLINE];
        if (strlen(prefix) == 0)
        {
            sprintf(prompt, "[%s]> ", cwd);
        }
        else
        {
            sprintf(prompt, "%s [%s]> ", prefix, cwd);
        }
        input = readline(prompt);
        if (input == NULL)
        {
            printf("Exiting...\n");
            break;
        }
        if (strlen(input) > 0)
        {
            add_history(input);
        }

        // tokenize
        int argIndex = 0;
        token = strtok(input, " ");
        while (token != NULL && argIndex < MAXARGS - 1)
        {
            args[argIndex++] = token;
            token = strtok(NULL, " ");
        }
        args[argIndex] = NULL;

        if (args[0] == NULL) // empty input
        {
            free(input);
            continue;
        }

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
            if (argIndex == 1)
            {
                list_dirs(".");
            }
            int count = 1;
            while (count < argIndex)
            {
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
                chdir(strcat(cwd, "/.."));
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
        else if (strcmp(args[0], "printenv") == 0)
        {
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
        }
        else if (strcmp(args[0], "setenv") == 0)
        {
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
                execlp(input, input, (char *)0);
                printf("couldn't execute: %s\n", input);
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

        free(input);
    }
    free(cwd);

    return 0;
}

void initialize_readline()
{
    rl_attempted_completion_function = command_completion;
}

char **command_completion(const char *text, int start, int end)
{
    if (strlen(text) == 0 && start == 0)
    {
        rl_attempted_completion_over = 1;
        return NULL;
    }

    if (start == 0)
    {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, rl_filename_completion_function);
}

char *command_generator(const char *text, int state)
{
    static int list_index, len;
    static const char *commands[] = {
        "exit",
        "which",
        "list",
        "pwd",
        "cd",
        "pid",
        "prompt",
        "printenv",
        "setenv",
        NULL};

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    const char *name;
    while ((name = commands[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }

    return NULL;
}

int is_executable(char *directory, char *command)
{
    char full_path[MAX_PATH_LENGTH];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, command);

    struct stat inputfer;
    if (stat(full_path, &inputfer) == 0 && inputfer.st_mode & S_IXUSR)
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

void print_env(char **envp)
{
    if (envp != NULL)
    {
        for (char **env = envp; *env != 0; env++)
        {
            char *thisEnv = *env;
            if (getenv(thisEnv) != NULL)
            {
                printf("%s=%s\n", thisEnv, getenv(thisEnv));
            }
            else
            {
                printf("No variable: %s\n", thisEnv);
            }
        }
    }
    else
    {
        printf("No environment variables found\n");
    }
}