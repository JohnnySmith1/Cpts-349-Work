#include "ucode.c"

#define BUF_SIZE 1024

main(int argc, char *argv[])
{
	char buf[BUF_SIZE];
	int numRead;


	if(argc == 3) // l2u f1 f2 case
	{
		int inputFd, outputFd;
		char *inputFile = argv[1];
		char *outputFile = argv[2];

		inputFd = open(inputFile, 0);
		creat(outputFile);
		outputFd = open(outputFile, 1);

		numRead = read(inputFd, buf, BUF_SIZE);
		

		while(numRead > 0)
		{
			char bufCopy[BUF_SIZE];

			strcpy(bufCopy, buf);

			ConvertToUpper(bufCopy);

			write(outputFd, bufCopy, numRead);
			numRead = read(inputFd, buf, BUF_SIZE);
		}

	}
	else if(argc == 1) // just l2u case
	{
		// We read from stdin and write to stdout in this case
		// gwrite(2, "reading from stdin...\n", 22);
		numRead = read(0, buf, BUF_SIZE);
		// gwrite(2, "read some bytes!\n", 17);

		while(numRead > 0)
		{
			char bufCopy[BUF_SIZE];

			strcpy(bufCopy, buf);

			ConvertToUpper(bufCopy);

			gwrite(1, bufCopy, numRead);

			// gwrite(2, "reading from stdin...\n", 22);
			numRead = read(0, buf, BUF_SIZE);
			// gwrite(2, "read some bytes!\n", 17);
		}
	}
	else
	{
		printf("l2u error: wrong parameters.\n");
		return;
	}

	exit(0);
}

int gabeIsLower(char c)
{
	int status = 0;

	if(c >= 'a' && c <= 'z')
	{
		status = 1;
	}

	return status;
}

char gabeToUpper(char c)
{
	if(gabeIsLower(c))
	{
		c -= 32;
	}
	return c;
}

int ConvertToUpper(char *buf)
{
	int i;
	
	for(i = 0; i < strlen(buf); i++)
	{
		buf[i] = gabeToUpper(buf[i]);
	}

	return 0;
}
