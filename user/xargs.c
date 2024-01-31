#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include <stdbool.h>

#define DEFAULT_COMMAND "echo"
#define MAX_ARG_LENGTH 64 // Maximum length of a single argument
#define EOF_CHAR (char)-1

/**
 * Copies `count` strings from `src` into `dest`. Only copies the pointers!
 */
void copy_string_array(char **dest, char **src, int count)
{
    while (count > 0)
    {
        *dest = *src;
        dest++;
        src++;
        count--;
    }
}

/**
 * Skips whitespace from `stdin`. Returns the first character it sees from stdin that
 * isn't whitespace. Returns `EOF_CHAR` if EOF reached.
 */
char skip_whitespace()
{
    int read_result = sizeof(char);
    char last_char = '\0';
    while (read_result == sizeof(char) && (last_char == ' ' || last_char == '\n'))
    {
        read_result = read(0, &last_char, sizeof(char));
    }
    if (read_result == 0)
    {
        return EOF_CHAR;
    }
    return last_char;
}

/**
 * Gets input from `stdin` until it sees whitespace (space, newline or EOF).
 * Returns the whitespace that stopped it.
 */
char scan_until_whitespace(char *dest)
{
    char *current_char = dest;

    // char first_char = skip_whitespace();
    // if (first_char == EOF_CHAR)
    // {
    //     *dest = '\0';
    //     return EOF_CHAR;
    // }
    // *current_char = first_char;
    // current_char++;

    int read_result = sizeof(char);
    char last_char = 0;

    while (read_result == sizeof(char) && !(last_char == ' ' || last_char == '\n'))
    {
        read_result = read(0, &last_char, sizeof(char));
        *current_char = last_char;
        current_char++;
    }

    if (last_char == ' ' || last_char == '\n')
    {
        *(current_char - 1) = '\0';
        return last_char;
    }

    *current_char = '\0';
    return EOF_CHAR;
}

/**
 * Deallocate the strings in the array. Expects a null-terminated array.
 */
void deallocate_string_array(char **array)
{
    char **current = array;
    while (*current != (char *)0)
    {
        free(*current);
        current++;
    }
}

/**
 * Executes a command by reading a line from `stdin` and parsing it as
 * whitespace-seperated arguments. Passes `existing_args` to the executable as well.
 * Expects `existing_args` to be a NULL-terminated array.
 * Returns `true` if EOF was reached, `false` otherwise.
 */
bool execute_line_command(char *args[])
{
    // Find the end of existing_args
    char **existing_args_end = args;
    while (*existing_args_end != (char *)0)
    {
        existing_args_end++;
    }

    char **current_arg = existing_args_end;

    char last_whitespace = ' ';
    while (last_whitespace == ' ')
    {
        char arg_buffer[MAX_ARG_LENGTH];
        last_whitespace = scan_until_whitespace(arg_buffer);
        if (arg_buffer[0] == '\0')
        {
            continue;
        }
        char *arg = (char *)malloc(sizeof(char) * (strlen(arg_buffer) + 1));
        strcpy(arg, arg_buffer);
        *current_arg = arg;
        current_arg++;
    }
    if (current_arg == existing_args_end)
    {
        return EOF_CHAR;
    }
    *current_arg = (char *)0;

    int fork_pid = fork();
    if (fork_pid == 0)
    {
        exec(args[0], args);
        exit(0);
    }
    else if (fork_pid > 0)
    {
        wait(0);
    }
    else
    {
        printf("xargs: fork error.\n");
        deallocate_string_array(existing_args_end);
        exit(1);
    }

    deallocate_string_array(existing_args_end);
    *existing_args_end = (char *)0;
    return last_whitespace == EOF_CHAR;
}

int main(int argc, char *argv[])
{
    char *args[MAXARG + 1];
    if (argc == 1)
    {
        args[0] = DEFAULT_COMMAND;
    }
    else
    {
        copy_string_array(args, argv + 1, argc - 1);
    }
    args[argc] = (char *)0;

    bool eof_reached = false;
    while (!eof_reached)
    {
        eof_reached = execute_line_command(args);
    }
    exit(0);
}