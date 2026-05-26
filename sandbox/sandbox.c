#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

//static pid_t child_pid;

//this is to take action but made the signal void like setting the ph on vibrate mode 
void alarm_handler(int sig)
{
    (void)sig;
}

//f is the function we are supposed to execute in the sandbox
//timeout is for the max execute time
//verbose is the boolean value to set true or false to print message 
/*
//this is the structure of sigaction 
struct sigaction {
    void     (*sa_handler)(int);  // Function to call
    sigset_t   sa_mask;           // Signals to block during handler
    int        sa_flags;          // Options (SA_RESTART, etc.)
    // ... other fields
};*/

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    struct sigaction sa; //to create an object of signal 
    pid_t pid; //to fork a process 
    int status; //to track the status of the child process 

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0; //0 means no special behavior to be taken
    sigemptyset(&sa.sa_mask); //this is to clear the signal mask meaning - no signals would be blocked 
    sigaction(SIGALRM, &sa, NULL);
    /*SIGALRM → Which signal to handle

&sa → Our settings structure

NULL → Don't need old settings saved */

    pid = fork();
    //fork failed and we return -1 for error in sandbox 
    if(pid == -1)
        return (-1);
    //if it is the child process , we will execute the function passed and exit with 0 means success 
    if( pid == 0)
    {
        f();
        exit(0);
    }
   // child_pid = pid;
   //parent process turn
   //we will set the alarm with the max execution time provided 
    alarm(timeout);
    //we will wait for the child to exit first 
    if(waitpid(pid, &status, 0) == -1) //the return value is -1 when the time passed and the signal interrupted the pid 
    {
        if(errno == EINTR) //this means the error is the interrupted system call 
        {
            kill(pid, SIGKILL); //we will force kill the child 
            waitpid(pid, NULL, 0); //wait for child to die //to prevent zombie
            if(verbose) //if the bool is true then display
                printf("Bad function: timed out after %u seconds\n", timeout);
            return(0); //bad function 
        }
        return (-1);
    }

    //this is to check for normal exit 
    if(WIFEXITED(status)) //to check if the child exited normally
    {
        if(WEXITSTATUS(status) == 0) //if the status is 0 
        {
            if(verbose)
                printf("Nice function!\n");
            return (1);
        }
        else
        {
            if(verbose)
                printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
            return (0);
        }
    }
    if(WIFSIGNALED(status)) //if the child process is killed by the signal
    {
        if(verbose)
            printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
        return(0);
    }
    return (-1);
}
