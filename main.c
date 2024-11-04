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

#include "builtin.h"

#define MAXLINE 128
#define MAXARGS 10
#define MAX_PATH_LENGTH 1024

int last_exit_status = 0;
char *cwd = NULL;
char prefix[MAXLINE] = "";
char *shell_name = NULL;

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
    "addacc",
    NULL};

char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);
void initialize_readline();

void setup_signal_handlers()
{
    struct sigaction sa;

    // Ignore SIGINT (Ctrl-C)
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // Ignore SIGTSTP (Ctrl-Z)
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // Ignore SIGTERM
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
}

// Function to execute a single command line
int execute_command(char *line, char **envp)
{
    // check for conditional execution
    int conditional = 0;
    if (line[0] == '?')
    {
        conditional = 1;
        line++;
    }

    if (conditional && last_exit_status != 0)
    {
        return 0;
    }

    char *args[MAXARGS];
    char *token;
    int argIndex = 0;
    pid_t pid;
    int status;
    int background = 0;
    glob_t glob_result;

    // Tokenize the line into args[]
    token = strtok(line, " ");
    while (token != NULL && argIndex < MAXARGS - 1)
    {
        if (strcmp(token, "&") == 0)
        {
            background = 1;
        }
        else if (strncmp(token, "$?", 2) == 0) // Replace $? with last exit status
        {
            char *status_str = malloc(12); // Enough for int
            sprintf(status_str, "%d", last_exit_status);
            args[argIndex++] = status_str;
        }
        else if (strncmp(token, "$0", 2) == 0 && token[2] == '\0') // IDK why but strcmp wasn't working
        {
            args[argIndex++] = strdup(shell_name);
        }
        else if (token[0] == '$') // Handle variable substitution
        {

            char *var_name = token + 1; // Skip the '$'
            char *value = getenv(var_name);
            if (value == NULL)
            {
                value = "";
            }
            args[argIndex++] = strdup(value);
        }
        else if (glob(token, GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result) == 0)
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

    if (args[0] == NULL) // Empty input
    {
        return 0;
    }

    char *noecho = getenv("NOECHO");
    // Check if it's a built-in command
    if (run_builtin(args, argIndex, envp) == 0)
    {
        for (int i = 0; i < argIndex; i++)
        {
            free(args[i]);
        }
        return 0;
    }

    // Handle "cd" command
    if (strcmp(args[0], "cd") == 0)
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in cd\n");
        }
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
            char *parent_dir = malloc(strlen(cwd) + 4); // Allocate enough space for cwd + "/.."
            if (parent_dir == NULL)
            {
                perror("malloc");
            }
            else
            {
                snprintf(parent_dir, strlen(cwd) + 4, "%s/..", cwd);
                if (chdir(parent_dir) == -1)
                {
                    perror("cd");
                }
                free(parent_dir);
            }
        }
        else
        {
            if (chdir(args[1]) == -1)
            {
                perror("cd");
            }
        }
        free(cwd); // Free old cwd
        cwd = getcwd(NULL, 0);
        if (cwd == NULL)
        {
            perror("getcwd");
            exit(1);
        }
        for (int i = 0; i < argIndex; i++)
        {
            free(args[i]);
        }
        return 0;
    }

    // Handle "prompt" command
    if (strcmp(args[0], "prompt") == 0)
    {
        if (!noecho || strlen(noecho) == 0)
        {
            printf("Executing built-in prompt\n");
        }
        if (argIndex == 2)
        {
            strcpy(prefix, args[1]);
            if (prefix[strlen(prefix) - 1] == '\n')
                prefix[strlen(prefix) - 1] = 0; // Replace newline with null
        }
        else
        {
            printf("Enter new prompt: ");
            fgets(prefix, MAXLINE, stdin);
            if (prefix[strlen(prefix) - 1] == '\n')
                prefix[strlen(prefix) - 1] = 0; // Replace newline with null
        }
        for (int i = 0; i < argIndex; i++)
        {
            free(args[i]);
        }
        return 0;
    }

    // Execute external commands
    if (strchr(args[0], '/') != NULL)
    {
        // Absolute or relative path
        if (access(args[0], X_OK) == 0)
        {
            if ((pid = fork()) < 0)
            {
                perror("fork");
                exit(1);
            }
            else if (pid == 0)
            {
                execv(args[0], args);
                printf("%s: Command not found.\n", args[0]);
                exit(127);
            }
            if (!background)
            {
                if ((pid = waitpid(pid, &status, 0)) < 0)
                    perror("waitpid");
                else
                {
                    last_exit_status = WEXITSTATUS(status);
                    if (last_exit_status != 0)
                    {
                        printf("Process exited with status %d\n", last_exit_status);
                    }
                }
            }
            else
            {
                // Handle background process
                // Save pid somewhere for later
            }
        }
        else
        {
            printf("File not found or not executable\n");
        }
    }
    else
    {
        // Command without path, use execvp
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            execvp(args[0], args);
            printf("%s: Command not found.\n", args[0]);
            exit(127);
        }
        if (!background)
        {
            if ((pid = waitpid(pid, &status, 0)) < 0)
                perror("waitpid");
            else
            {
                last_exit_status = WEXITSTATUS(status);
                if (last_exit_status != 0)
                {
                    printf("Process exited with status %d\n", last_exit_status);
                }
            }
        }
        else
        {
            // Handle background process
            // Save pid somewhere for later
        }
    }
    for (int i = 0; i < argIndex; i++)
    {
        free(args[i]);
    }
    return 0;
}

void initialize_readline()
{
    rl_attempted_completion_function = command_completion;
}

char **command_completion(const char *text, int start, int end)
{
    (void)end;
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

int main(int argc, char *argv[], char **envp)
{
    char *input;

    initialize_readline();
    setup_signal_handlers();

    shell_name = argv[0];

    if (getenv("ACC") == NULL)
    {
        setenv("ACC", "0", 1);
    }

    cwd = getcwd(NULL, 0);
    if (cwd == NULL)
    {
        perror("getcwd");
        exit(1);
    }

    if (argc > 1)
    {
        char *file_path = argv[1];
        while (isspace((unsigned char)*file_path))
            file_path++;
        if (*file_path == 0)
        {
            fprintf(stderr, "Error: Empty file path\n");
            exit(1);
        }
        char *end = file_path + strlen(file_path) - 1;
        while (end > file_path && isspace((unsigned char)*end))
            end--;
        end[1] = '\0';

        FILE *file = fopen(argv[1], "r");
        if (file == NULL)
        {
            perror("fopen");
            exit(1);
        }

        char line[MAXLINE];
        while (fgets(line, sizeof(line), file))
        {
            // Remove trailing newline
            line[strcspn(line, "\n")] = '\0';

            if (strlen(line) == 0)
                continue;

            if (strncmp(line, "prompt", 6) == 0)
            {
                continue;
            }

            // printf("Executing: %s\n", line);
            int ret = execute_command(line, envp);
            if (ret == 1) // Exit command received
                break;
        }

        fclose(file);
        free(cwd);
        return last_exit_status;
    }

    const char *color_start = "\033[1;32m"; // Green color
    const char *color_end = "\033[0m";      // Reset color

    printf("Welcome to the Gage Shell!\n");
    while (1)
    {
        size_t prompt_size = strlen(prefix) + strlen(cwd) + strlen(color_start) + strlen(color_end) + 6; // 6 for extra characters
        char *prompt = malloc(prompt_size);
        if (prompt == NULL)
        {
            perror("malloc");
            exit(1);
        }
        if (strlen(prefix) == 0)
        {
            snprintf(prompt, prompt_size, "%s[%s]> %s", color_start, cwd, color_end);
        }
        else
        {
            snprintf(prompt, prompt_size, "%s%s [%s]> %s", color_start, prefix, cwd, color_end);
        }

        input = readline(prompt);
        free(prompt);

        if (input == NULL)
        {
            printf("\n");
            continue;
        }

        if (strlen(input) > 0)
        {
            add_history(input);
        }

        int ret = execute_command(input, envp);
        free(input);
        if (ret == 1) // Exit command received
            break;
    }
    free(cwd);

    return 0;
}
