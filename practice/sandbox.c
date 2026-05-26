#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

//this function will be executed when the sigalarm is caught
void alarm_handler(int sig)
{
    (void)sig; //this is to prevent variable not used 
}

//this function will run the passed function based on the allowed time out
//the boolean value is to confirm whether or not the text will be displayed

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    //as we will need a signal to catch (sigalarm) we will create one
    //we will be needing a process to fork so that we can run the function passed 
    //the status variable is to catch the status of the child process which is used to run the function passed

    struct sigaction sa;
    pid_t pid; 
    int status;

    //we will be assigning the value to signal
    //according to the subject, we will run the function we created when the signal alarm is caught. 
    //thus, set sa_handler
    //then we don't have any special conditions for this signal thus, we will set sa_flags to be 0
    //then, we won't be blocking any of the signals thus set sa_mask to empty
    //then, we will assign the signal we created to SIGALARM to catch that

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    //not fork the process to run the function if it is a child 
    pid = fork();
    if (pid == -1)
        return -1;

    if (pid == 0)
    {
        f();
        exit(0);
    }

    //if it is a parent process , we will set the parent process
    // then, we will need to check the status of the child process and if the timeout has passed or not by checking the interrupted signal 
    //if there is a interrupted signal, we will kill the child process immediately
  //then wait for the child to die and if the bool is true, we will display the msg. 
    alarm(timeout);
    if (waitpid(pid,  &status, 0) == -1)
    {
        if (errno == EINTR)
        {
            kill(pid, SIGKILL);
            waitpid (pid, NULL, 0);
            if (verbose)
                printf("Bad function : timed out after %u seconds \n", timeout);
            return (0);
        }
        return (-1);
    }

    //now if the status of the child process is ok, we will check the if the process has exited or not
    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0)
        {
            if (verbose)
                printf("Nice function\n");
            return (1);
        }
        else
        {
            if (verbose)
                printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
            return (0);
        }
    }
    if (WIFSIGNALED(status))
    {
        if (verbose)
            printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
        return (0);
    }
    return (-1);
} 