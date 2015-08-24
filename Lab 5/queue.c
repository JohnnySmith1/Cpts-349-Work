void do_kfork()
{
	fork();
}

void do_tswitch()
{
	printf("in do_tswitch()\n");
	running->status = READY;
	tswitch();
}

void do_ps()
{
	int i = 0, index = 0;
	char *p, buf[16];
	int pid, ppid;
	char status[10];
	strcpy(status, "         ");
	// write C code to print ALL PROC's pid, ppid, status;
	printf("==================================================\n");
	printf("  %s        %s      %s      %s\n", "da name", "status", "pid", "ppid");
	printf("--------------------------------------------------\n");
	for(i = 0; i < 9; i++)
	{
		strcpy(status, "         ");
		status[9] = 0;
		switch(proc[i].status)
		{
			case 0:
				strncpy(status, "FREE", 4);

				break;
			case 1:
				strncpy(status, "READY", 5);
				break;
			case 2:
				strncpy(status, "RUNNING", 7);
				break;
			case 3:
				strncpy(status, "STOPPED", 7);
				break;
			case 4:
				strncpy(status, "SLEEP", 5);
				break;
			case 5:
				strncpy(status, "ZOMBIE", 6);
				break;
		}
		p = proc[i].name;
		strcpy(buf, "               ");
		index = 0;
		while(*p) 
		{
			buf[index] = *p;
			index++;
			p++;
		}
		printf("%s  %s", buf, status);
		if(proc[i].status != 0)
		{
			printf("      %u", proc[i].pid);
			printf("      %u", proc[i].ppid);
		}
		printf("\n");
	}
	
	return;
}

void do_wakeup()
{
	char s[10];
	int event;
	printf("Enter a wake up value: ");
	gets(s);
	event = atoi(s);
	kwakeup(event);
	return;
}

void do_sleep()
{
	// ask for an event (value), e.g. 123; 
	int event;
	char s[10];
	printf("Enter a value to sleep on: ");
	gets(s);
	event = atoi(s);

	ksleep(event);
	return;
}

void do_wait()
{
	int pid, status;
	pid = kwait(&status);
	// print pid and status;
	if(pid != 0)
	{
		printf("Wait pid: %d\n", pid);	
	}
}

void do_exit()
{
	char s[10];
	int exitValue;
		
	// ask for an event (value);
	int event = 0;
	printf("Enter an event value to kill the proc on: ");
	// write YOUR gets(char s[ ]) function to return s;
	gets(s);
	exitValue = atoi(s);
	// We call kexit(exitValue) to DIE:
	kexit(exitValue);
}

