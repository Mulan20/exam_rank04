#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

void alarm_handler(int sig)
{
    void(sig);
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    sig_t sigaction sa;
    int status;
    pid_t pid;

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    emptyset(&sa.sa_masks);
    sigaction(SIGALRM, &sa, NULL);

    pid = fork();
    if (pid == -1)
        return -1;

    if (pid == 0)
    {
        f();
        exit(0);
    }

    alarm(timeout);
    if (waitpid(pid, &signal, 0) == -1)
    {
        if (errno = EINTR)
        {
            kill(pid, SIGKILL);
            wait(pid, NULL, 0);
            if (verbose)
                printf("Bad function: timed out after %u seconds!\n", timeout);
            return (0);
        }
        return (-1);
    }

    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0)
        {
            if (verbose)
                printf("nice function!\n");
            return(1);
        }
        else
        {
            if (verbose)
                printf("bad function: child exited with %d\n!", WEXITSTATUS(status));
            return (0);
        }
    }

    if (WIFSIGNALED(status))
    {
        if (verbose)
            printf("bad function : %s\n", strsignal(WTERMSIG(status)));
        return (0);
    }
    return (-1);
}