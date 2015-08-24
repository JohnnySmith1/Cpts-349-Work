int pid, status, serial0Child = 0, serial1Child = 0, consoleChild = 0;
int stdin, stdout;

#include "ucode.c"

main(int argc, char *argv[])
{
	// 1. // open /dev/tty0 as 0 (READ) and 1 (WRITE) in order to display messages
	open("/dev/tty0", 0);
	open("/dev/tty0", 1);

	// 2. // Now we can use printf, which calls putc(), which writes to stdout
	printf("GABE INIT FORKS A LOGIN TASK\n");

	serial0Child = fork();

	if (serial0Child == 0)
	{
		// consoleChild executes this
		// Child sets up serial terminal here
		exec("login /dev/ttyS0");
	}
	else
	{
		serial1Child = fork();

		if(serial1Child == 0)
		{
			// Child sets up other serial terminal here
			exec("login /dev/ttyS1");
		}
		else
		{
			// Parent finally starts the qemu console with stuff
			consoleChild = fork();

			if(consoleChild == 0)
			{
				// console child executes this
				login();
			}
			else
			{
				// Parent executes this
				parent();
			}
		}
	}

	gwrite(2, "init is exiting...\n", 19);
	exit(0);
	
	
}

int login()
{
	exec("login /dev/tty0");
}


int parent()
{
	while(1)
	{
		printf("GABE INIT WAITS\n");

		pid = wait(&status);


		if (pid == consoleChild)
		{
			// fork another login child
			consoleChild = fork();

			login();
		}
		else
		{
			printf("GABE INIT : buried an orphan child %d\n", pid);
		}
	}
}