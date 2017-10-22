#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main (void)
{
	int i;
	int fd;
	char* mem;
	
	if ((fd = open ("temp", O_RDWR)) < 0)  {
        perror ("open error");
		return 1;
    }

    mem = mmap (0, 10, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);

    if (mem == MAP_FAILED) {
        perror ("mmap error2:");
		return 1;
    }
	
	for (i = 0; i < 5; i++) {
		char temp;
		
		temp = mem [i];
		mem [i] = mem [9 - i];
		mem [9 - i] = temp;
	}
	
	munmap (mem, 10);
	close (fd);

    return 1;
}

