#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int p[2];
    char data[1];
    pipe(p);
    int pid = fork();
    if (pid > 0) // Parent
    {
        data[0] = 'a';
        write(p[1], data, 1);
        wait(0);
        read(p[0], data, 1);
        printf("%d: received pong\n", getpid(), data[0]);
        close(p[1]);
    }
    else if (pid == 0) // Child
    {
        read(p[0], data, 1);
        printf("%d: received ping\n", getpid(), data[0]);
        write(p[1], data, 1);
        close(p[0]);
    }
    else
    {
        printf("fork error.\n");
        exit(1);
    }

    exit(0);
}