#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5





typedef struct proc
{
	struct proc *next;
	int    *ksp;
	int    pid;                // add pid for identify the proc
	int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
	int    ppid;               // parent pid
	struct proc *parent;
	int    priority;
	int    event;
	int    exitCode;
	int    kstack[SSIZE];      // per proc stack area
} PROC;

int procSize = sizeof(PROC);
int nproc = 0;
int color;
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

#include "queue.c"
#include "wait.c"
#include "kernel.c"






int init()
{
	PROC *p;
	int i;

	printf("init ....");

	for (i=0; i<NPROC; i++){   // initialize all procs
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;     
		p->next = &proc[i+1];
	}
	freeList = &proc[0];      // all procs are in freeList
	proc[NPROC-1].next = 0;
	readyQueue = sleepList = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);
	p->status = RUNNING;
	running = p;
	nproc++;
	printf("init done\n");
}

int scheduler()
{
	if (running->status == READY)
	{
		printf("enqueuing in scheduler: %u", running->pid);
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	running->status = 2;
	printf("new running->pid: %d\n", running->pid);
	color = 0x000A + (running->pid % 6);
}

int body();

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

	printf("enqueuing in kfork: %u\n", p->pid);
	// enter p into readyQueue;
	enqueue(&readyQueue, p);




	return p;
}

int body()
{
	char c;
	printf("proc %d resumes to body()\n", running->pid);
	while(1)
	{
		printf("-----------------------------------------\n");
		//print freeList;
		printf("Free List:\n");
		printQueue(freeList);
		// print readyQueue;
		printf("Ready Queue:\n");
		printQueue(readyQueue);
		// print sleepList;
		printf("Sleep List:\n");
		printSleepQueue(sleepList);
		printf("-----------------------------------------\n");

		printf("proc %d[%d] running: parent=%d\n",
			running->pid, running->priority, running->ppid);

		printf("type a char [s|f|q| p|z|a| w ] : ");
		c = getc(); 
		printf("%c\n", c);

		switch(c){
			case 's' : do_tswitch();   break;
			case 'f' : do_kfork();     break;
			case 'q' : do_exit();      break; 
			case 'p' : do_ps();        break; 
			case 'z' : do_sleep();     break; 
			case 'a' : do_wakeup();    break; 
			case 'w' : do_wait();      break;
		}
	}
}





main()
{
	printf("MTX starts in main()\n");
	init();      // initialize and create P0 as running
	kfork();     // P0 kfork() P1
	while(1)
	{
		printf("P0 running\n");
		if (nproc==2 && proc[1].status != READY)
		{
			printf("no runable process, system halts\n");
		}
		while(!readyQueue);
		running->status = READY;
		printf("P0 switch process\n");
		tswitch();   // P0 switch to run P1
	}
	
}







/***********************************************************
Write YOUR C code for
ksleep(), kwakeup()
kexit()
kwait()

Then, write your C code for
do_ps(), do_sleep(), do_wakeup(), do_wait()
************************************************************/
