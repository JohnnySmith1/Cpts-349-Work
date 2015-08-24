#include "ucode.c"

#define BUF_SIZE 1024
#define SCREEN_SIZE 25

char fgetc(int fd);

main(int argc, char *argv[])
{
	char buf[BUF_SIZE], temp[50], pipeDev[40];
	int pfd;

	if(argc == 2) // Regular case, aka [more filename]
	{
		int fd, numRead;
		int index, lineCount;
		char *filename = argv[1];

		fd = open(filename, 0); // open for read

		numRead = read(fd, buf, BUF_SIZE);
		
		lineCount = 0;

		while(numRead > 0)
		{
			for(index = 0; index < strlen(buf); index++)
			{
				if(buf[index] == '\n')
				{
					lineCount++;
					gwrite(1, &buf[index], 1);
					
					if(lineCount == SCREEN_SIZE - 1)
					{
						char input;

						input = getc();

						if(input == ' ')
						{
							// We go a full page down
							lineCount = 0;
						}
						else
						{
							// We go one space down
							lineCount = SCREEN_SIZE - 2;
						}
					}

				}
				else
				{
					gwrite(1, &buf[index], 1);
				}
				
			}

			numRead = read(fd, buf, BUF_SIZE);
		}

	}
	else if(argc == 1) // more used in pipe, aka [ more ]
	{
		// NOTE: this will not work for input I/O redirection, aka [more < file1]
		// This is because the input for more gets changed to the file

		int fd, numRead;
		int index, lineCount;

		sprintf(temp, "%d", pfd);
		gwrite(2, temp, strlen(temp));

		numRead = read(0, buf, BUF_SIZE);
		gettty(pipeDev);
		pfd = open(pipeDev, O_RDONLY);
		
		
		
		lineCount = 0;

		while(numRead > 0)
		{
			for(index = 0; index < strlen(buf); index++)
			{
				if(buf[index] == '\n')
				{
					lineCount++;
					gwrite(1, &buf[index], 1);
					
					if(lineCount == SCREEN_SIZE - 1)
					{
						char input;

						
						input = fgetc(pfd);

						if(input == ' ')
						{
							// We go a full page down
							lineCount = 0;
						}
						else
						{
							// We go one space down
							lineCount = SCREEN_SIZE - 2;
						}
					}
				}
				else
				{
					gwrite(1, &buf[index], 1);
				}
				
			}

			numRead = read(0, buf, BUF_SIZE);
		}
	}
	


	
	
}





char fgetc(int fd)
{
	int n;
	char c;

	n = read(fd, &c, 1);

	if(n == 0 || c == 4)  // c is the EOF character, or ctrl+d
	{
		return -1;
	}

	return c;
}