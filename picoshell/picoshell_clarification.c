#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int picoshell(char **cmds[])
{
	int ret = 0;
	pid_t pid;
	int fd[2];
	int last_fd = -1;
	int i = 0; // iterator 
	int status;
	
	while (cmds[i])
	{
		if (cmds[i + 1])
		{
			if (pipe(fd) == -1)
			{
				if (last_fd != -1)
					close(last_fd);
				return (1);
			}
			
		}
		
		pid = fork();
		if (pid == -1)
		{
			if (cmds[i + 1])
			{
				close(fd[0]);
				close(fd[1]);
			}
			if (last_fd != -1)
				close(last_fd);
			return (1);
		}
		
		if (pid == 0)
		{
			if (last_fd != -1)
			{
				if (dup2(last_fd, STDIN_FILENO) == -1)
					exit(1);
				close(last_fd);
			}
			
			if (cmds[i + 1])
			{
				close(fd[0]);
				if (dup2(fd[1], STDOUT_FILENO) == -1)
					exit(1);
				close(fd[1]);
			}
			execvp(cmds[i][0], cmds[i]);
			exit(1);
		}
		//this is for parent process
		if (last_fd != -1)
		{
			close(last_fd);
		}
		if (cmds[i + 1])
		{
			close(fd[1]);
			last_fd = fd[0];
		}
		else
			last_fd = -1;
		i++;
		
	}
	if (last_fd != -1)
		close(last_fd);
		
	while (wait(&status) > 0)
	{
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			ret = 1;
	}
	
	return (ret);
}
