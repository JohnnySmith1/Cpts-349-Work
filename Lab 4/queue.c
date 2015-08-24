// void do_ps();
// void do_wakeup();
// void do_sleep();
// void do_wait();
// void do_exit();
// void printQueue(PROC *queue);
// PROC *get_proc();
// void put_proc(PROC *p);
// PROC *dequeue(PROC **queue);
// void enqueue(PROC **queue, PROC *p);
// PROC *kfork();


PROC *kfork(char *filename);

void do_kfork()
{
	kfork("/bin/u1");
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

/************* LAB 2 Code here *************/
void printQueue(PROC *queue)
{
	// print the queue entries in [pid, priority]->  format;
	PROC *temp = queue;
	while(temp != 0)
	{
		printf("[%u, %u] -> ", temp->pid, temp->priority);
		temp = temp->next;
	}
	printf("[NULL]\n");
}

void printSleepQueue(PROC *queue)
{
	// print the sleep queue entries in [pid, priority]->  format;
	PROC *temp = queue;
	while(temp != 0)
	{
		printf("%u [event = %x] -> ", temp->pid, temp->event);
		temp = temp->next;
	}
	printf("[NULL]\n");
}

PROC *dequeue(PROC **queue)
{
	// remove a PROC with the highest priority (the first one in queue)
	// return its pointer;
	PROC *dequeuedProc = (*queue);
	(*queue) = (*queue)->next;
	return dequeuedProc;
}

PROC *get_proc()
{
	// get a FREE PROC from freeList; return PROC pointer; 
	// return 0 if no more FREE PROCs.
	return dequeue(&freeList);
}

void put_proc(PROC *p)
{
	// Appends p to the end of freeList
	PROC *temp;
	temp = freeList;

	while(temp->next != 0)
	{
		temp = temp->next;
	}

	temp->next = p;
}

void enqueue(PROC **queue, PROC *p)
{
	// enter p into queue BY PRIORITY
	PROC *prev = 0, *curr = 0;
	if(p == 0)
	{
		printf("input p is null.\n");
		return;
	}


	curr = (*queue);
	while(curr != 0 && curr->priority >= p->priority)
	{
		prev = curr;
		curr = curr->next;
	}

	if(curr == 0 && prev == 0)
	{
		(*queue) = p;
		(*queue)->next = 0;
	}
	else if(prev == 0)
	{
		p->next = (*queue);
		(*queue) = p;
	}
	else if(curr != 0 && prev != 0)
	{
		prev->next = p;
		p->next = curr;
	}
	else
	{
		// end case
		prev->next = p;
		p->next = 0;
	}


	return;
}



