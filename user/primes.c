#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime_sieve(int fd_in)
{
    int prime;
    int read_result = read(fd_in, &prime, sizeof(int));

    if (read_result == 0)
    {
        return;
    }

    printf("%d prime\n", prime);

    int prime_pipe[2];
    pipe(prime_pipe);

    if (fork() > 0)
    {
        int i;
        while (read(fd_in, &i, sizeof(int)) != 0)
        {
            if (i % prime != 0)
            {
                write(prime_pipe[1], &i, sizeof(int));
            }
        }
        close(prime_pipe[1]);
        wait(0);
    }
    else
    {
        close(prime_pipe[1]);
        prime_sieve(prime_pipe[0]);
    }
}

int main(int argc, char  argv[]) {
    int main_pipe[2];
    pipe(main_pipe);

    if (fork() > 0)
    {
        for (int i = 2; i < 35; ++i)
        {
            write(main_pipe[1], &i, sizeof(int));
        }
        close(main_pipe[1]);
        wait(0);
    }
    else
    {
        close(main_pipe[1]);
        prime_sieve(main_pipe[0]);
    }
    return 0;
}
