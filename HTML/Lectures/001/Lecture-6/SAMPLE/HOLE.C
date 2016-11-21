#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

char buf1 [] = "abcdefghij";
char buf2 [] = "ABCDEFGHIJ";

void err_sys (const char* info)
{
	perror (info);
	exit (1);
}

int main (void)
{
	int fd;

	if ( (fd = creat ("file.hole", 0644)) < 0)
		err_sys ("create error");
	
	if (write (fd, buf1, 10) != 10)
		err_sys ("buf1 write error");
	
	if (lseek (fd, 40960, SEEK_SET) == -1)
		err_sys ("lseek error");
	
	if (write (fd, buf2, 10) != 10)
		err_sys ("buf2 write error");
	
	exit (0);
}

