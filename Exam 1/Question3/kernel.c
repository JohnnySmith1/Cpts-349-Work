#define NPROC 9
// kernel.c file:

// YOUR kernel.c file as in LAB3
int ksleep(int event)
{
	running->event = event;      //Record event in PROC
	running->status = SLEEP;     // mark itself SLEEP
	// For fairness, put running into a FIFO sleepList so that they will wakeup in order
	enqueue(&sleepList, running);
	tswitch();                   // not in readyQueue anymore
}

int kwakeup(int event)
{
	int i, found;
	PROC *curr, *prev;

	for(i = 0; i < 9; i++)
	{
		if(proc[i].status==SLEEP && proc[i].event==event)
		{
			curr = 0;
			prev = 0;
			found = 0;
			curr = sleepList;

			while(curr != 0)
			{
				if(curr->pid == proc[i].pid)
				{
					found = 1;
					break;
				}
				prev = curr;
				curr = curr->next;
			}

			if(prev == 0 && curr == 0)
			{
				// we do nothing, because the sleepList was empty to begin with
				printf("SleepList was empty\n");
			}
			else if(prev == 0 && curr != 0 && found==1)
			{
				// It's the beginning of the sleepList
				// printf("Found at the beginning of sleepList.\n");
				sleepList = sleepList->next;
			}
			else if(prev != 0 && curr != 0 && found==1)
			{
				// Case where it's in the middle of the sleepList
				// printf("Found in the middle or end of sleepList.\n");
				prev->next = curr->next;
			}
			else if(prev != 0 && curr == 0 && found==0)
			{
				// Case where it's at the end of the sleepList
				printf("Didn't find any matching procs.\n");
			}

			if(found)
			{
				proc[i].status = READY;
				enqueue(&readyQueue, curr);
			}
			
		}
	}
	return 0;
}


int prockexit(int exitvalue, int pidNum)
{
	int i, count, foundChildren;
	PROC *temp, *procToDelete;

	procToDelete = readyQueue;
	while(procToDelete != 0)
	{
		if(procToDelete->pid = pidNum)
		{
			printf("Found P%u to delete\n", procToDelete->pid);
			break;
		}
		procToDelete = procToDelete->next;
	}

	temp = readyQueue;
	foundChildren = 0;

	// Record value in its PROC.exitValue;
	procToDelete->exitCode = exitvalue;
	// Give away children (dead or alive) to P1. 
	while(temp != 0)
	{
		if(temp->ppid == running->pid)
		{
			// Sets the temp's parent id to P1
			foundChildren = 1;
			temp->ppid = 1;
		}
		temp = temp->next;
	}
	// Make sure P1 does not die if other procs still exist.
	temp = readyQueue;

	count = 0;
	if(procToDelete->pid == 1)
	{
		while(temp != 0)
		{
			count++;
			temp = temp->next;
		}
		if(count >= 1)
		{
			printf("Cannot kill P1!\n");
			return 0;
		}
		
	}

	// Issue wakeup(parent) to wake up its parent;
	kwakeup(&proc[procToDelete->ppid]);
	// Wake up P1 also if it has sent any children to P1;
	if(foundChildren)
	{
		kwakeup(&(proc[1]));
	}
	

	printf("Marking proc %u as ZOMBIE\n", procToDelete->pid);
	// Mark itself a ZOMBIE;
	procToDelete->status = ZOMBIE;
	
	// dequeue from readyQueue
	procDequeue(procToDelete->pid);
}

int procDequeue(int pid)
{
	PROC *curr = 0, *prev = 0;

	curr = readyQueue;

	while(curr != 0)
	{
		if(curr->pid == pid)
		{
			printf("Found P%u to dequeue!!\n", curr->pid);
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	if(prev == 0)
	{
		// first item in readyQueue needs to be dequeued
		printf("Case 1!!!\n");
		readyQueue = curr->next;
	}
	else if(prev != 0 && curr != 0)
	{
		// item not in the beginning needs to be dequeued
		printf("Case 2!!!\n");
		prev->next = curr->next;
	}

	return 0;
}


int kexit(int exitvalue)
{
	int i, count, foundChildren;
	PROC *temp;

	temp = readyQueue;
	foundChildren = 0;

	// Record value in its PROC.exitValue;
	running->exitCode = exitvalue;
	// Give away children (dead or alive) to P1. 
	while(temp != 0)
	{
		if(temp->ppid == running->pid)
		{
			// Sets the temp's parent id to P1
			foundChildren = 1;
			temp->ppid = 1;
		}
		temp = temp->next;
	}
	// Make sure P1 does not die if other procs still exist.
	temp = readyQueue;

	count = 0;
	if(running->pid == 1)
	{
		while(temp != 0)
		{
			count++;
			temp = temp->next;
		}
		if(count >= 1)
		{
			printf("Cannot kill P1!\n");
			return 0;
		}
		
	}

	// Issue wakeup(parent) to wake up its parent;
	kwakeup(&proc[running->ppid]);
	// Wake up P1 also if it has sent any children to P1;
	if(foundChildren)
	{
		kwakeup(&(proc[1]));
	}
	

	// Mark itself a ZOMBIE;
	running->status = ZOMBIE;
	// Call tswitch(); to give up CPU;
	tswitch();

}

int kwait(int *status)
{
	int i;
	int foundChildren;
	int childPid;
	PROC *deadChildProc;

	i = 0;
	deadChildProc = 0;
	foundChildren = 0;

	// We look to make sure there's children of this proc
	for(i = 0; i < 9; i++)
	{
		if(proc[i].ppid == running->pid)
		{
			foundChildren = 1;
			break;
		}
	}


	if(foundChildren != 1)
	{	
		// We cannot run wait on this proc without it having children, so we return
		printf("Wait error: Didn't find any children. \n");
		return 0;
	}

	while(1)
	{
		for(i = 0; i < 9; i++)
		{
			// Check for a dead child of the running proc
			if((proc[i].ppid == running->pid) && (proc[i].status == ZOMBIE))
			{
				printf("Found dead child proc.\n");
				deadChildProc = &proc[i];
				break;
			}
		}

		if(deadChildProc != 0)
		{
			// copy child's exitValue to *status
			*status = deadChildProc->exitCode;
			// save its pid
			childPid = deadChildProc->pid;
			// free the ZOMBIE child PROC
			deadChildProc->status = FREE;
			// (enter it into freeList)
			enqueue(&freeList, deadChildProc);
			// return deadchild's pid
			return childPid;
		}

		printf("Didn't find dead child proc. Putting proc to sleep...\n");
		// sleep at its own &PROC
		// Addresses are numbers, so we can easily pass the running pointer
		//  as an event number
		ksleep(running);
	}


	return 0;

}

PROC *kfork(char *filename)
{
	// kfork as BEFORE: pid = new PROC's pid
	PROC *p;
	PROC *temp;
	int i, j, segment;
	// (1). PROC *p = get_proc(); to get a FREE PROC from freeList;
	// if none, return 0 for FAIL;
	// TODO: figure out running status change problem
	p = get_proc();

	if(p == 0)
	{
		printf("get_proc() failed, no more PROCs available\n");
	}

	// (2). Initialize the new PROC p with
	// --------------------------
	// status   = READY;
	// priority = 1;
	// ppid = running pid;
	// parent = running;
	// --------------------------
	p->status = READY;
	p->priority = 1;
	p->ppid = running->pid;
	p->parent = running;
	// *********** THIS IS THE MAIN PART OF THE ASSIGNMENT!!!***********
	// INITIALIZE p's kstack to make it start from body() when it runs.
	// To do this, PRETEND that the process called tswitch() from the 
	// the entry address of body() and executed the SAVE part of tswitch()
	// to give up CPU before. 
	// Initialize its kstack[ ] and ksp to comform to these.
	for(j = 1; j < 10; j++)
	{
		p->kstack[SSIZE - j] = 0;
	}
	p->kstack[SSIZE - 1] = (int)body;
	p->ksp = &(p->kstack[SSIZE-9]);

	printf("p->status while enqueueing in kfork(): %u\n", p->status);
	// enter p into readyQueue;
	enqueue(&readyQueue, p);


	// new PROC's segment = (pid+1)*0x1000;
	nproc++;
	if(filename) 
	{
		segment = 0x1000*(p->pid+1);
		// This line loads the file
		// (4).1    SETUP new PROC's ustack for it to return to Umode to execute filename;
		load(filename, segment);
		// the whole segment is 64 KB.

		// this zeroes out everybody.
		for(i = 1; i <= 12; i++) {
			put_word(0, segment, -2*i);
		}

		// This is for the flag
		put_word(0x0200, segment, -2);
		// uDS 
		put_word(segment, segment, -2*12);
		// uES,
		put_word(segment, segment, -2*11);
		// and uCS
		put_word(segment, segment, -2*2);




		// we save the segment into the proc's slot
		// PROC.usp = TOP of ustack (per INT 80)
		p->usp = -2 * 12;
		// Set new PROC.uss = segment;
		p->uss = segment;
	}

	// return pointer to new PROC;
	return p;
}

