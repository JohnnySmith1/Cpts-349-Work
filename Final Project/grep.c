#include "ucode.c"

#define BUF_SIZE 256

main(int argc, char *argv[])
{
	char buf[BUF_SIZE];

	if(argc == 3) // grep used regular way aka [grep pattern file]
	{
		int fd, numRead;
		char *pattern = argv[1];
		char *filename = argv[2];

		fd = open(filename, 0); // open for read

		numRead = read(fd, buf, BUF_SIZE);
		

		while(numRead > 0)
		{
			char *token;
			char bufCopy[BUF_SIZE];

			strcpy(bufCopy, buf);
			token = strtok(bufCopy, "\n");

			while(token != 0)
			{
				if(strContains(pattern, token))
				{
					gwrite(1, token, strlen(token));
					gwrite(1, "\n", 1);
				}

				token = strtok(0, "\n");
			}

			

			numRead = read(fd, buf, BUF_SIZE);
		}

	}
	else if(argc == 2) // Grep used in pipe aka [grep pattern]
	{
		int fd, numRead;
		char *pattern = argv[1];

		numRead = read(0, buf, BUF_SIZE);
		

		while(numRead > 0)
		{
			char *token;
			char bufCopy[BUF_SIZE];

			strcpy(bufCopy, buf);
			token = strtok(bufCopy, "\n");

			while(token != 0)
			{
				if(strContains(pattern, token))
				{
					gwrite(1, token, strlen(token));
					gwrite(1, "\n", 1);
				}

				token = strtok(0, "\n");
			}

			

			numRead = read(fd, buf, BUF_SIZE);
		}
	}
	else
	{
		printf("grep error: wrong parameters.\n");
		
	}

	exit(0);
}


int strContains(char *pattern, char *str)
{
	int i, patternIndex;
	int status = 0;
	int patternLength;

	patternLength = strlen(pattern);

	for(i = 0; i < strlen(str) - patternLength; i++)
	{
		int mismatch = 0;
		for(patternIndex = 0; patternIndex < patternLength; patternIndex++)
		{
			if(pattern[patternIndex] != str[patternIndex + i])
			{
				mismatch = 1;
				break;
			}
		}

		if(mismatch == 0)
		{
			// We found the pattern, so we break out of loop and return
			status = 1;
			break;
		}
	}

	return status;
}