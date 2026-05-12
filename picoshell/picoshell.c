#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

// ls -l | grep ".c" | wc -l this is what picoshell is trying to do 
int picoshell(char **cmds[])
{
    pid_t pid; //to create a process
    int fd[2]; //to create two pipe ends
    int last_fd = -1; //to store the previous pipe 
    int i = 0; //counter i
    int status; //to store child exit status 
    int ret = 0;

    //we will loop while there are commands 
    while (cmds[i])
    {
        // if we see there is a following command, we will create the pipe 
        if (cmds[i + 1])
        {
            //if pipe creation failed, return error
            if (pipe(fd) == -1)
                return (1);
        }
        
        //we will create a process
        pid = fork();
        //if the process creation failed 
        if (pid == -1)
        {
            //if the process failed and there is still the following command (meaning there was a pipe created)
            if (cmds[i + 1])
            {
                //then we must close the pipe 
                close(fd[0]);
                close(fd[1]);
            }
            //return error
            return (1);
        }
        
        //if it is a child process
        if (pid == 0)
        {
            //if there was a last pipe end  meaning not the first command 
            if (last_fd != -1)
            {
                //we will try to set that pipe end to be the input end (redirection fo the pipe end)
                if (dup2(last_fd, STDIN_FILENO) == -1)
                    exit(1);
                close(last_fd); //close the original end 
            }
            //if there is a following command (not the last comand)
            if (cmds[i + 1])
            {
                //we will close the input end 
                close(fd[0]);
                //we will set the write end of the pipe 
                if (dup2(fd[1], STDOUT_FILENO) == -1)
                    exit(1);
                //then close the write end
                close(fd[1]);
            }
            //we will try to execute the command 
            execvp(cmds[i][0], cmds[i]);
            exit(1);
        }
        //the following code runs in the parent process 
        //if there is no previous pipe end
        //Parent only needs to keep the current pipe's read end for the next command.
        if (last_fd != -1)
            close(last_fd);
        // if there is one more command afterwards 
        if (cmds[i + 1])
        {
            close(fd[1]); //parent doesn't need write end
            last_fd = fd[0];
        }
        else
        {
            last_fd = -1;  // FIX #3: Reset for last command
        }
        i++;
    }
    
    //if any pipe still open, close it
    if (last_fd != -1)
        close(last_fd);
    
    // we will need to wait until all the processes have sucessfully executed 
    while (wait(&status) > 0)
    {
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            ret = 1;
    }
    
    return (ret);
}
