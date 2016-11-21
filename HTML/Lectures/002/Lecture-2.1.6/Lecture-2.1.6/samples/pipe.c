#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define MAXLINE 256

void err_sys (const char* info)
{
	perror (info);
	exit (1);
}

int main (void)
{
    int n, fd [2];
    pid_t pid;
    char line [MAXLINE];

    if (pipe (fd) < 0)
         err_sys ("pipe error");
       
    if ( (pid = fork ()) < 0)
         err_sys ("fork error");

    else if (pid > 0) { // parent
         close (fd [0]);
         write (fd [1], "hello world\n", 12);
    }
    else {
         close (fd [1]);
         n = read (fd[0], line, MAXLINE);
         write (STDOUT_FILENO, line, n);
    }

    exit (0);
}

