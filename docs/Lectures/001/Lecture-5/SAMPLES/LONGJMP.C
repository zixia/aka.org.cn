#include <stjmp.h>

jmp_buf   jmpbuffer;

int main (void)
{

    char line [MAXLINE];

	switch (setjmp (jmpbuffer)) {
		case 0:
			break;
		case 1:
			break;
		default:
			break;
	}

	return 0;
}

void some_func (void)
{
	if (...)
		longjmp (jmpbuffer, 0);
	else if (...)
		longjmp (jmpbuffer, 1);
	else
		longjmp (jmpbuffer, 2);
}


