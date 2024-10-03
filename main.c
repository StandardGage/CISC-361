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
#include <glob.h>

#define MAXLINE 128
#define MAXARGS 10
#define MAX_PATH_LENGTH 1024

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

char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);
void initialize_readline();

// parse command entered
int main(int argc, char *argv[], char **envp)
{

    char *input;
    pid_t pid;
    int status;
    int background;
    char *args[MAXARGS];
    char *token;
    char *cwd = getcwd(NULL, 0);
    char prefix[MAXLINE] = "";

    background = 0;

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
        glob_t glob_result;
        while (token != NULL && argIndex < MAXARGS - 1)
        {
            if (glob(token, GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result) == 0)
            {
                for (size_t i = 0; i < glob_result.gl_pathc; i++)
                {
                    args[argIndex++] = strdup(glob_result.gl_pathv[i]);
                }
                globfree(&glob_result);
            }
            else
            {
                args[argIndex++] = strdup(token);
            }
            token = strtok(NULL, " ");
        }
        args[argIndex] = NULL;

        if (args[0] == NULL) // empty input
        {
            free(input);
            continue;
        }

        // check if args[0] is a built-in command
        if (run_builtin(args, argIndex, envp) == 0)
        {
            free(input);
            continue;
        }
        else if (strcmp(args[0], "cd") == 0) // cd command needed here to update cwd
        {
            printf("Executing built-in cd\n");
            if (argIndex == 1)
            {
                if (chdir(getenv("HOME")) == -1)
                {
                    perror("cd");
                }
            }
            else if (argIndex > 2)
            {
                printf("Too many arguments\n");
            }
            else if (strcmp(args[1], "-") == 0)
            {
                if (chdir(strcat(cwd, "/..")) == -1)
                {
                    perror("cd");
                }
            }
            else
            {
                if (chdir(args[1]) == -1)
                {
                    perror("cd");
                }
            }
            cwd = getcwd(NULL, 0);
        }
        else if (strcmp(args[0], "prompt") == 0) // prompt command needed here to update prompt/prefix
        {
            printf("Executing built-in prompt\n");
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
        // check if absolute path or relative path
        else if (strchr(args[0], '/') != NULL)
        {
            if (access(args[0], X_OK) == 0)
            {
                if ((pid = fork()) < 0)
                {
                    printf("fork error\n");
                    exit(1);
                }
                else if (pid == 0)
                {
                    execv(args[0], args);
                    printf("couldn't execute: %s\n", args[0]);
                    exit(127);
                }

                if (!background)
                {
                    printf("Executing %s\n", args[0]);
                    if ((pid = waitpid(pid, &status, 0)) < 0)
                        printf("waitpid error\n");
                }
                else
                {
                    printf("Executing %s\n", args[0]);
                    // save pid somewhere for later
                }
            }
            else
            {
                printf("File not found or not executable\n");
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
                execvp(args[0], args);
                printf("couldn't execute: %s\n", args[0]);
                exit(127);
            }

            if (!background)
            {
                printf("Executing %s\n", args[0]);
                if ((pid = waitpid(pid, &status, 0)) < 0)
                    printf("waitpid error\n");
            }
            else
            {
                printf("Executing %s\n", args[0]);
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