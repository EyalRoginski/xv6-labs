#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX 35

void prime_sieve(int first, int fd_in, int fd_out)
{
    int n = first;
    // printf("prime %d\n", n);

    int p;
    while (read(fd_in, (char *)&p, sizeof(int)) != 0)
    {
        if (first == 2)
        {
            // printf("fd_in: %d, read p: %d\n", fd_in, p);
        }
        if (p % n != 0)
        {
            write(fd_out, (char *)&p, sizeof(int));
        }
    }
    // printf("prime sieve finished on pid %d\n", getpid());
}

void prime_sieve_level(int fd_in)
{
    int first = 0;
    int read_result = read(fd_in, &first, sizeof(int));
    if (read_result == 0)
    {
        printf("end.\n");
        close(fd_in);
        exit(0);
    }

    int out_pipe[2];
    pipe(out_pipe);

    int pid = fork();

    if (pid > 0)
    {
        printf("%d\n", pid);
        prime_sieve(first, fd_in, out_pipe[1]);
        close(out_pipe[1]);
        close(fd_in);
        wait(0);
    }
    else if (pid == 0)
    {
        prime_sieve_level(out_pipe[0]);
    }
    else
    {
        printf("fork error.\n");
        exit(1);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    int main_pipe[2];
    pipe(main_pipe);

    int pid = fork();

    if (pid > 0)
    {
        for (int i = 2; i <= MAX; i++)
        {
            write(main_pipe[1], (char *)&i, sizeof(int));
        }
        close(main_pipe[1]);
        wait(0);
    }
    else if (pid == 0)
    {
        prime_sieve_level(main_pipe[0]);
    }
    else
    {
        printf("fork error.\n");
        exit(1);
    }

    exit(0);
}