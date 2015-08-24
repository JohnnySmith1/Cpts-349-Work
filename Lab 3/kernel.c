#define NPROC 9



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
				printf("SleepList was empty of sleepList.\n");
			}
			else if(prev == 0 && curr != 0 && found==1)
			{
				// It's the beginning of the sleepList
				printf("Found at the beginning of sleepList.\n");
				sleepList = sleepList->next;
			}
			else if(prev != 0 && curr != 0 && found==1)
			{
				// Case where it's in the middle of the sleepList
				printf("Found in the middle or end of sleepList.\n");
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
		if(count >= 2)
		{
			printf("Cannot kill P1! Other procs still exist.\n");
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