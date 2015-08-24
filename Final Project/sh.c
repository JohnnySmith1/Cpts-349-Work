int status, pid;

#include "ucode.c"
#define BUF_LEN 1024



main(int argc, char *argv[])
{
	char command[500];
	char directory[500];
	char buf[256];

	// gwrite(2, "sh is exiting!!\n", 16);
	// exit(0);

	printf("GASH # : ");
	gets(command);
	
	while(strcmp(command, "logout") != 0)
	{
		if(ContainsPipes(command))
		{
			pid = fork();
			if(pid == 0)
			{
				// Child executes this
				HandlePipes(command);
			}
			else
			{
				// Parent executes this
				pid = wait(&status);
				// printf("Handle pipes returned!\n");
			}
		}
		else if(ContainsIORedirection(command))
		{
			HandleIORedirection(command);
		}
		else if(ContainsCDCommand(command))
		{
			FindDirectory(command, directory);
			printf("Changing directory to: %s\n", directory);

			chdir(directory);
			
		}
		else if(strcmp(command, "pwd") == 0)
		{
			getcwd(buf);
			printf("%s\n", buf);
		}
		else
		{
			// if just ONE cmd:
			// fork() returns 0 if child, non-zero if parent
			pid = fork();
			if (pid == 0)
			{
				// Child executes this
				exec(command);
			}		
			else
			{
				// Parent executes this
				pid = wait(&status);
			}
		}

		printf("GASH # : ");
		gets(command);
	}

	// gwrite(2, "sh is exiting!\n", 15);
	// syscall to die;
	exit(0);

	
	
}

int HandleIORedirection(char *command)
{
	int IOType;

	// 1 = output >
	// 2 = input <
	// 3 = append >>
	IOType = GetIOType(command);

	switch(IOType)
	{
		case 1:
			HandleOutputRedirection(command);
			break;
		case 2:
			HandleInputRedirection(command);
			break;
		case 3:
			HandleAppendRedirection(command);
			break;
		default:
			printf("Unknown IO redirection\n");
			break;
	}
}

int HandleOutputRedirection(char *command)
{
	int i, numTokens, destFileFd, outputPid, outputStatus;
	char *token;
	char tokens[20][50], lhsCommand[BUF_LEN], destFile[50], commandCopy[BUF_LEN];
	STAT destFileStatBuf;


	// Clear the tokens array
	for(i = 0; i < 20; i++)
	{
		strcpy(tokens[i], "");
	}

	strcpy(commandCopy, command);

	token = strtok(commandCopy, " ");
	i = 0;
	numTokens = 0;
	while(token != 0)
	{
		strcpy(tokens[i], token);
		i++;
		numTokens++;

		token = strtok(0, " ");
	}

	// We now have the tokens handled
	// We know that the output operator will always be the second token from the right
	// We also know that the furthest right token is the file
	strcpy(destFile, tokens[numTokens - 1]);

	stat(destFile, &destFileStatBuf);
	if(destFileStatBuf.st_ino == 0)
	{
		// File doesn't exist, so we create it
		creat(destFile);
	}

	// We build the exec string for the LHS
	// We build up all the left-hand side tokens in a loop but the last one, 
	// putting spaces inbetween them
	strcpy(lhsCommand, "");
	for(i = 0; i < numTokens - 3; i++)
	{
		strcat(lhsCommand, tokens[i]);
		strcat(lhsCommand, " ");
	}
	// We finally strcat on the last command, because we don't want a space after it
	strcat(lhsCommand, tokens[i]);

	

	// We fork a process to handle the left hand side command
	// fork() returns 0 if child, non-zero if parent
	outputPid = fork();
	if (outputPid == 0)
	{
		// We close stdout, because when we open the destFile, 
		// it will take stdout's fd (aka 1)
		close(1);

		// We open the destination file for write mode
		destFileFd = open(destFile, 1);

		// Child executes this
		exec(lhsCommand);
	}		
	else
	{
		// Parent executes this
		outputPid = wait(&outputStatus);
	}
	

	return 0;
}

int HandleAppendRedirection(char *command)
{
	int i, numTokens, destFileFd, appendPid, appendStatus;
	char *token;
	char tokens[20][50], lhsCommand[BUF_LEN], destFile[50], commandCopy[BUF_LEN];
	STAT destFileStatBuf;


	// Clear the tokens array
	for(i = 0; i < 20; i++)
	{
		strcpy(tokens[i], "");
	}

	strcpy(commandCopy, command);

	token = strtok(commandCopy, " ");
	i = 0;
	numTokens = 0;
	while(token != 0)
	{
		strcpy(tokens[i], token);
		i++;
		numTokens++;

		token = strtok(0, " ");
	}

	// We now have the tokens handled
	// We know that the append operator will always be the second token from the right
	// We also know that the furthest right token is the file
	strcpy(destFile, tokens[numTokens - 1]);

	stat(destFile, &destFileStatBuf);
	if(destFileStatBuf.st_ino == 0)
	{
		// File doesn't exist, so we create it
		creat(destFile);
	}

	// We build the exec string for the LHS
	// We build up all the left-hand side tokens in a loop but the last one, 
	// putting spaces inbetween them
	strcpy(lhsCommand, "");
	for(i = 0; i < numTokens - 3; i++)
	{
		strcat(lhsCommand, tokens[i]);
		strcat(lhsCommand, " ");
	}
	// We finally strcat on the last command, because we don't want a space after it
	strcat(lhsCommand, tokens[i]);


	// We fork a process to handle the left hand side command
	// fork() returns 0 if child, non-zero if parent
	appendPid = fork();
	if (appendPid == 0)
	{
		// We close stdout, because when we open the destFile, 
		// it will take stdout's fd (aka 1)
		close(1);

		// We open the destination file for write mode
		destFileFd = open(destFile, O_WRONLY|O_CREAT|O_APPEND);

		// Child executes this
		exec(lhsCommand);
	}
	else
	{
		// Parent executes this
		appendPid = wait(&appendStatus);
	}
	

	return 0;
}

int HandleInputRedirection(char *command)
{
	int i, numTokens, destFileFd, inputPid, inputStatus;
	char *token;
	char tokens[20][50], lhsCommand[BUF_LEN], sourceFile[50], commandCopy[BUF_LEN];
	STAT destFileStatBuf;


	// Clear the tokens array
	for(i = 0; i < 20; i++)
	{
		strcpy(tokens[i], "");
	}

	strcpy(commandCopy, command);

	token = strtok(commandCopy, " ");
	i = 0;
	numTokens = 0;
	while(token != 0)
	{
		strcpy(tokens[i], token);
		i++;
		numTokens++;

		token = strtok(0, " ");
	}

	// We now have the tokens handled
	// We know that the input operator will always be the second token from the right
	// We also know that the furthest right token is the file
	strcpy(sourceFile, tokens[numTokens - 1]);

	stat(sourceFile, &destFileStatBuf);
	if(destFileStatBuf.st_ino == 0)
	{
		// We just return with an error if the input file doesn't exist
		printf("Error: Input file doesn't exist.\n");
		return -1;
	}

	// We build the exec string for the LHS
	// We build up all the left-hand side tokens in a loop but the last one, 
	// putting spaces inbetween them
	strcpy(lhsCommand, "");
	for(i = 0; i < numTokens - 3; i++)
	{
		strcat(lhsCommand, tokens[i]);
		strcat(lhsCommand, " ");
	}
	// We finally strcat on the last command, because we don't want a space after it
	strcat(lhsCommand, tokens[i]);

	

	// We fork a process to handle the left hand side command.
	// fork() returns 0 if child, non-zero if parent.
	inputPid = fork();
	if (inputPid == 0)
	{
		// We close stdin, because when we open the sourceFile, 
		// it will take stdin's fd (aka 0)
		close(0);

		// We open the destination file for write mode
		destFileFd = open(sourceFile, 0);

		// Child executes this
		exec(lhsCommand);
	}		
	else
	{
		// Parent executes this
		inputPid = wait(&inputStatus);
	}
	

	return 0;
}

int isWhiteSpace(char c)
{
	int status = 0;
	if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
	{
		status = 1;
	}
	return status;
}

int trimWhiteSpace(char *str)
{
	char buf[BUF_LEN];
	int endIndex, i, bufIndex;
	char c;

	if(str == 0)
	{
		printf("trimError: str is null.\n");
		return -1;
	}

	for(i = 0; i < BUF_LEN; i++)
	{
		buf[i] = 0;
	}
	

	// We chop off the left and the right hand side white space.

	// Start with the left side
	// Find first index of no white space
	i = 0;
	c = str[0];
	while(isWhiteSpace(c))
	{
		i++;
		c = str[i];
	}

	// Copy over until c is '\0'
	bufIndex = 0;
	while(c != 0)
	{
		buf[bufIndex] = c;
		bufIndex++;
		i++;
		c = str[i];
	}


	endIndex = strlen(buf) - 1;

	c = buf[endIndex];

	while(isWhiteSpace(c))
	{
		buf[endIndex] = 0;
		endIndex--;
		c = buf[endIndex];
	}

	// Finally, we copy over the trimmed string to the original string
	strcpy(str, buf);

	return 0;
}

int HandlePipes(char *command)
{
	int i, r, numCommands, pd[2], pid, status;
	char *token;
	char commands[5][BUF_LEN], commandCopy[BUF_LEN];
	char lhsCommand[BUF_LEN], rhsCommand[BUF_LEN];
	char lhsCopy[500], rhsCopy[500], debug[100];

	strcpy(commandCopy, command);

	// strtok command by | symbols
	token = strtok(commandCopy, "|");

	i = 0;
	numCommands = 0;
	while(token != 0)
	{
		strcpy(commands[i], token);
		i++;
		numCommands++;
		token = strtok(0, "|");
	}

	// We build up left hand side of pipe
	// Copy over first command token from left hand side
	// We know there are at least two command tokens in a pipe
	strcpy(lhsCommand, commands[0]);

	// Continue copying over all the commands besides the last one, putting
	// the pipe symbols back
	for(i = 1; i < numCommands - 1; i++)
	{
		strcat(lhsCommand, "|");
		strcat(lhsCommand, commands[i]);
	}


	// We might have spaces at the beginning or end, we trim it off
	trimWhiteSpace(lhsCommand);

	strcpy(rhsCommand, commands[numCommands - 1]);
	trimWhiteSpace(rhsCommand);
	

	// create pipe
	r = pipe(pd);


	// fork child
	pid = fork();

	if(pid)
	{
		// Parent executes this
		// This proc will be READING from the pipe
		close(pd[1]);
		close(0);
		dup2(pd[0], 0);

		// We DO NOT call wait(&status) here, because the reader and writer will simultaneously write to the pipe and read things.
		// If we waited() here, the writer could only write so many bytes to the pipe, then it would go to sleep, and the 
		// program will hang.
		

		
		// The right hand side may still contain an I/O redirection
		if(ContainsIORedirection(rhsCommand))
		{
			HandleIORedirection(rhsCommand);
		}
		else
		{
			// If it is just a basic command, we go ahead and execute it
			exec(rhsCommand);
		}

		
		
	}
	else
	{
		// child as the pipe WRITER
		close(pd[0]);
		close(1);
		dup2(pd[1], 1);

		// if there is a | symbol in the lefthand side, recursively call HandlePipes(lhsCommand);
		if(ContainsPipes(lhsCommand))
		{
			HandlePipes(lhsCommand);
		}
		else if(ContainsIORedirection(lhsCommand))
		{
			HandleIORedirection(lhsCommand);
		}
		else
		{
			// Execute for the left-hand side command with exec()
			exec(lhsCommand);
		}


	}

	return 0;
}

int FindDirectory(char *command, char *directory)
{
	char buf[1024], lines[10][100], commandCopy[500];
	int passwordFileFd = 0, i = 0, linesCount = 0;
	char *token, *lineToken;

	// If they only typed "cd" or "cd ", then the strlen is 3 or less
	if(strlen(command) <= 3)
	{
		// We have to open the passwd file up and find the user's home directory
		passwordFileFd = open("/etc/passwd", 0);

		read(passwordFileFd, buf, 1024);

	    // We first read in all the lines
		i = 0;
		token = strtok(buf, "\n");


		while(token != 0)
		{
			strcpy(lines[i], token);
			i++;
			linesCount++;
			token = strtok(0, "\n");
		}

	    // We then tokenize each line, one at a time
		i = 0;

		while(i != linesCount)
		{
			lineToken = strtok(lines[i], ":"); // username
			lineToken = strtok(0, ":");  // password
			lineToken = strtok(0, ":"); // gid
			lineToken = strtok(0, ":"); // uid

			// 6. Check if user account is valid
			if(atoi(lineToken) == getuid())
			{
				// We found a matching user account!
				lineToken = strtok(0, ":"); // fullname
				lineToken = strtok(0, ":"); // HOMEDIR
				strcpy(directory, lineToken);
				break;
			}

			i++;
		}

		close(passwordFileFd);
	}
	else
	{
		strcpy(directory, command + 3);
	}

	
	return 0;
}

int ContainsCDCommand(char *command)
{
	int status = 0;
	char buf[3];

	// Only need to copy over the first two characters
	buf[0] = command[0];
	buf[1] = command[1];
	buf[2] = 0;

	if(strcmp(buf, "cd") == 0)
	{
		status = 1;
	}

	return status;
}

int ContainsPipes(char *command)
{
	int found = 0;
	char commandCopy[50];
	char *temp;

	strcpy(commandCopy, command);
	temp = commandCopy;

	while(*temp != 0)
	{
		if(*temp == '|')
		{
			found = 1;
			break;
		}
		temp++;
	}
	return found;
}

int ContainsIORedirection(char *command)
{
	int status = 0, i = 0;

	while(command[i] != 0)
	{
		if(command[i] == '<' || command[i] == '>')
		{
			status = 1;
			break;
		}

		i++;
	}

	return status;
}

int GetIOType(char *command)
{
	int type = 0, i;

	for(i = 0; i < strlen(command); i++)
	{
		if(command[i] == '>')
		{
			// It might be output or append. We use the next char to determine which
			// We can safely increment i here even in a for loop because we have at least an output operator
			i++;

			if(command[i] == '>')
			{
				type = 3;
			}
			else
			{
				type = 1;
			}

			break;
		}
		else if(command[i] == '<')
		{
			type = 2;
			break;
		}

	}

  	// 1 = output >
	// 2 = input <
	// 3 = append >>
	return type;
}


