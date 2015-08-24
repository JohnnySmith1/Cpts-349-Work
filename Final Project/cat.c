#include "ucode.c"
#define BUF_SIZE 10

/*
NOTES: We have 2 cases:
	
	1. we cat out one or more files
	2. cat was called by itself because it is part of a pipe
*/

main(int argc, char *argv[])
{
	char buf[BUF_SIZE], debug[100];
	int numRead, totalRead;

	totalRead = 0;
	
	if(argc >= 2) // Case 1: cat one or more files
	{
		int fd, i;

		for(i = 1; i < argc; i++)
		{
			char *filename = argv[i];
			fd = open(filename, 0); // open for read

			numRead = read(fd, buf, BUF_SIZE);

			while(numRead > 0)
			{
				gwrite(1, buf, numRead);
				numRead = read(fd, buf, BUF_SIZE);
			}

			close(fd);
		}
	}
	else // Case 2: cat was called by itself in a pipe, or a situation like cat < myfile.txt
	{
		// We read from stdin and write to stdout in this case
		numRead = read(0, buf, BUF_SIZE);

		while(numRead > 0)
		{
			gwrite(1, buf, numRead);
			numRead = read(0, buf, BUF_SIZE);
		}

	}
	
	exit(0);
	
}
