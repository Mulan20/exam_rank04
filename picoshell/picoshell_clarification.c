#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int picoshell(char **cmds[])
{
    pid_t pid;
    int fd[2];
    int last_fd = -1;
    int i = 0;
    int status;
    int ret = 0; //consider success at first 

    while (cmds[i])
    {
        //we will need to create the pipe if there is a following command
        if (cmds[i + 1])
        {
            if (pipe(fd) == -1)
                return (1); //return fail
        }

        //now create the process
        pid = fork();
        //if the process creation failed
        if (pid == -1)
        {
            //if they have created the pipe as there is a following command, we must close that and return error
            if (cmd[i + 1])
            {
                close(fd[0]);
                close(fd[1]);
            }
            return (1);
        }
        //if it is a child process, we must execute the process but first check whether there was a previous command executed or not
        if (pid == 0)
        {
            //if there is a previous pipe, we will get the data from that so set that to be the read end of the new process
            if (last_fd != -1)
            {
                if (dup2(last_fd, STDIN_FILENO) == -1)
                    exit(1);
                close(last_fd);
            }
            //now check whether there are commands following
            if (cmds[i + 1])
            {
                //if so, we must close the read end as it is not necessary
                close(fd[0]);
                if (dup2(fd[1], STDOUT_FILENO) == -1)
                    exit(1);
                //close the original write end 
                close(fd[1]);
            }
            execvp(cmds[i][0], cmds[i]);
            exit(1);
        }

        //the following is about the parent process
        //the parent process runs immediately as the child process
        //its role is to clean and organize the pipe  

        //if there is a previous pipe end, we will close that 
        if (last_fd != -1)
            close(last_fd);
        //if there are more commands left
        if (cmds[i + 1])
        {
            close(fd[1]); //parents don't need to write any data into this 
            last_fd = fd[0]; //we set this as we want the child to read data from the previous pipe end
        }
        else
        {
            last_fd = -1;
        }
        i++;
    }

    //if there is no command and the last_fd is set, we must close that
    if (last_fd != -1)
        close(last_fd);

    //now, we must wait all the child processes to be executed and exited first
    while(wait(&status) > 0)
    {
        //the first condition = checks whether the child ends normally
        //the second condition = checkes the status of exit (0 for success)
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            ret = 1; //if the child processes didn't exit properly
    }

    return (ret);
}