#define NPROC 9
extern int goUmode();


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


int kexec(char filename[15])
{
	int segment, i;



	if(strcmp(filename, "/bin/u1") != 0 && strcmp(filename, "/bin/u2") != 0)
	{
		printf("File not found.\n");
		return -1;
	}

	segment = 0x1000*(running->pid+1);

	// We load the new file
	load(filename, segment);

	// We now fix the stack
	//  (LOW)  uSP                                | by INT 80  |   HIGH
 //     ---------------------------------------------------------------------
 //           |uDS|uES| di| si| bp| dx| cx| bx| ax|uPC|uCS|flag| XXXXXX
 //     ---------------------------------------------------------------------
 //            -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1 | 0 

 //        (a). re-establish ustack to the very high end of the segment.
 //        (b). "pretend" it had done  INT 80  from (virtual address) 0: 
 //             (c). fill in uCS, uDS, uES in ustack
 //             (d). execute from VA=0 ==> uPC=0, uFLAG=0x0200, 
 //                                        all other registers = 0;
 //             (e). fix proc.uSP to point at the current ustack TOP (-24)

 //     Finally, return from exec() ==> goUmode().

	// Reset all registers to 0.
	for(i = 1; i <= 12; i++)
	{
		put_word(0, segment, 2*-i);
	}

	// uCS
	put_word(segment, segment, 2*-2);
	// uDS
	put_word(segment, segment, 2*-12);
	// uES
	put_word(segment, segment, 2*-11);
	// uPC
	put_word(0, segment, 2*-3);
	// uFLAG
	put_word(0x0200, segment, 2*-1);
	// Correct usp to point to top of stack
	running->usp = 2*-12;

	return goUmode();
}




