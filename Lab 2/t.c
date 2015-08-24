/************ t.c file **********************************/
#define NPROC     9        
#define SSIZE  1024                /* kstack int size */

#define FREE      0                /* proc status     */
#define READY     1
#define SLEEP     2 				// waiting for a child to finish it's thing
#define BLOCK     3
#define ZOMBIE    4 				// inactive until the parent calls it

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

typedef struct proc{
	struct proc *next;
	int    ksp;

	int    status;       // FREE|READY|SLEEP|BLOCK|ZOMBIE
	int    priority;     // priority
	int    pid;          // process pid
	int    ppid;         // parent pid 
	struct proc *parent; // pointer to parent PROC

	int    kstack[SSIZE]; // SSIZE=1024
} PROC;




#include "io.c" /**** USE YOUR OWN io.c with myprintf() here *****/

PROC proc[NPROC], *running, *freeList, *readyQueue;

int  procSize = sizeof(PROC);

/****************************************************************
Initialize the proc's as shown:

running->proc[0]--> proc[1] --> proc[2] ... --> proc[NPROC-1] -->
^                                         |
|<---------------------------------------<------------

Each proc's kstack contains:
retPC, ax, bx, cx, dx, bp, si, di, flag;  all 2 bytes
*****************************************************************/

// Forward declarations to shut up the compiler
int body();
void enqueue(PROC **queue, PROC *p);
PROC *dequeue(PROC **queue);
PROC *kfork();

void printQueue(PROC *queue)
{
	// print the queue entries in [pid, priority]->  format;
	PROC *temp = queue;
	while(temp != 0)
	{
		myprintf("[%u, %u] -> ", temp->pid, temp->priority);
		temp = temp->next;
	}
	myprintf("[NULL]\n");
}

int initialize()
{
	int i, j;
	PROC *p;

	// (1). For each PROC do:
	//            status = FREE;
	//            pid = index of PROC
	//            priority = 0;
	//            ppid = 0; 
	//            parent = 0;
	//   (2). Let running->PROC[0]; status=READY; parent=&PROC[0];
	//   (3). freeList = P1 -> P2 ----> P8 -> 0
	//   (4). readyQueue = 0;
	for(i = 0; i < NPROC; i++)
	{
		proc[i].status = FREE;
		proc[i].pid = i;
		proc[i].priority = 0;
		proc[i].ppid = 0;
		proc[i].parent = 0;

		// Have to initilize the kstack for all procs except p0
		if(i)
		{
			for(j = 1; j < 10; j++)
			{
				p->kstack[SSIZE - j] = 0;
			}
			p->kstack[SSIZE - 1] = (int)body;
			p->ksp = &(p->kstack[SSIZE-9]);
		}
	}

	running = &proc[0];
	running->status = READY;
	running->parent = &proc[0];

	freeList = &proc[1];
	for(i = 2; i < NPROC; i++)
	{
		freeList->next = &proc[i];
		freeList = freeList->next;
	}
	// Make freeList end with a NULL
	freeList = 0;
	// Reassign freelist again to reset it back to the front
	freeList = &proc[1];

	readyQueue = 0;


}



void do_exit()
{
	running->status = ZOMBIE;
}

int body()
{
	char c;
	// 's' : call tswitch to switch process;
	// 'q' : call exit(), which changes running process status to ZOMBIE,
	//                  then call tswitch() to give up CPU;

	// 'f' : kfork() a child PROC;
	//     print message showing success or failure.
	
	while(1)
	{
		myprintf("proc %d resumes to body() function\n", running->pid);

		myprintf("**************************\n");
		myprintf("freeList: \n");
		printQueue(freeList);
		myprintf("readyQueue: \n");
		printQueue(readyQueue);
		myprintf("current running proc: [%u, %u]  parent: [%u, %u]\n", 
			running->pid, running->priority, running->parent->pid, running->parent->priority);
		myprintf("**************************\n");

		myprintf("I am Proc %d in body(): Enter a key : ", running->pid);
		c=getc(0);
		myprintf("\n\n\n");
		switch(c) 
		{
			case 's':
				tswitch();
				break;
			case 'q':
				myprintf("\nq entered.\n");
				myprintf("calling do_exit()...\n");
				do_exit();
				myprintf("calling tswitch()...\n");
				tswitch();
				break;
			case 'f':
				kfork();
				break;
			default:
				myprintf("Character not recognized.\n");
		}
	}
	
}

void scheduler()
{
	// Modified scheduler function
	if (running->status == READY)
	{
		myprintf("enqueing %u\n", running->pid);
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	myprintf("dequeued %u\n", running->pid);
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
	// TODO: redo this to handle by priority
	PROC *prev = 0, *curr = 0;
	if(p == 0)
	{
		myprintf("input p is null.\n");
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
		// We have to worry about extra stuff here
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

PROC *dequeue(PROC **queue)
{
	// remove a PROC with the highest priority (the first one in queue)
	// return its pointer;
	PROC *dequeuedProc = (*queue);
	(*queue) = (*queue)->next;
	return dequeuedProc;
}

PROC *kfork()
{
	PROC *p;
	PROC *temp;
	int j;
	// (1). PROC *p = get_proc(); to get a FREE PROC from freeList;
	// if none, return 0 for FAIL;
	p = get_proc();

	if(p == 0)
	{
		myprintf("get_proc() failed, no more PROCs available\n");
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

	myprintf("enqueuing %u\n", p->pid);
	// enter p into readyQueue;
	enqueue(&readyQueue, p);




	return p;
}

main()
{
	char c;
	myprintf("\nWelcome to the 460 Multitasking System\n");
	initialize();
	kfork();   // create a child process in readyQueue

	// loop forever
	while(1)
	{  
		if (readyQueue)
		{
			myprintf("calling tswitch in main: \n");
    		tswitch(); // switch process
    	}
    }
}



