extern int goUmode();
extern int body();


// your kernel.c file with
//kfork(), fork(), exec() functions
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

PROC *kfork(char *filename)
{
	// kfork as BEFORE: pid = new PROC's pid
	PROC *p;
	PROC *temp;
	int i, j, segment;
	// (1). PROC *p = get_proc(); to get a FREE PROC from freeList;
	// if none, return 0 for FAIL;



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
	p->kstack[SSIZE - 1] = (int)goUmode;
	p->ksp = &(p->kstack[SSIZE-9]);

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

int copy_image(u32 child_segment)
{
	// copies the Umode image of running to child_segment.
	// copy_image() implies that the Umode images of parent and child are
	//     IDENTICAL. So, their saved usp must also be the same value (each is an 
	//     offset relative to its own segment).
	u16 i, runningSegment;
	u16 tempWord;

	
	// runningSegment = 0x1000*(running->pid+1);
	runningSegment = running->uss;
	printf("!!! The child_segment: %x\n", child_segment);
	printf("!!! The runningSegment: %x\n", runningSegment);

	for(i = 0; i < (32*1024); i++)
	{
		tempWord = get_word(runningSegment, i*2);
		// printf("tempWord: %x\n", tempWord);
		// printf("tempOffset: %x\n", tempOffset);
		put_word(tempWord, (u16)child_segment, i*2);
	}
}


int fork()
{
	// kfork as BEFORE: pid = new PROC's pid
	PROC *p;
	PROC *temp;
	u16 tempWord;
	int i, j, segment, runningSegment;
	// (1). PROC *p = get_proc(); to get a FREE PROC from freeList;
	// if none, return 0 for FAIL;
	p = get_proc();

	if(p == 0)
	{
		printf("get_proc() failed, no more PROCs available\n");
		return 0;
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
	p->kstack[SSIZE - 1] = (int)goUmode;
	p->ksp = &(p->kstack[SSIZE-9]);

	// enter p into readyQueue;
	enqueue(&readyQueue, p);


	// new PROC's segment = (pid+1)*0x1000;
	nproc++;

	segment = 0x1000*(p->pid+1);



	// ****** NEW WAY ******
	printf("About to fork() the NEW way\n");
	copy_image(segment);



	// we save the segment into the proc's slot
	// PROC.usp = TOP of ustack (per INT 80)
	
	// Set new PROC.uss = segment;
	p->uss = segment;
	// usp is the only thing that is copied over from parent
	p->usp = running->usp;

	// uDS 
	put_word(segment, segment, p->usp);
	// uES,
	put_word(segment, segment, p->usp+(2*1));
	// and uCS
	put_word(segment, segment, p->usp+(2*10));
	// the flag
	put_word(0x0200, segment, p->usp+(2*11));



	// return new proc's pid;
	return p->pid;
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

int kps()
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
	
	return 0;
}

int kmode()
{
	// body();
	printf("in kmode\n");
	return 0;
}

int chname(char *y)
{
	// WRITE C CODE to change running's name string;
	char buf[64];
	char *cp = buf;
	int count = 0; 

	while (count < 16){
		*cp = get_byte(running->uss, y);
		if (*cp == 0) break;
		cp++; 
		y++; 
		count++;
	}
	buf[31] = 0;

	printf("changing name of proc %d to %s\n", running->pid, buf);
	strcpy(running->name, buf); 
	printf("done\n");
}