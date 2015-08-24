#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
"Saturn", "Uranus", "Neptune" };

/**************************************************
bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "queue.c"
#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file

int color;

int init()
{
	PROC *p; int i;
	color = 0x0C;
	printf("init ....");
	for (i=0; i<NPROC; i++)
	{   
		// initialize all procs
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;  
		strcpy(proc[i].name, pname[i]);
		p->next = &proc[i+1];
	}
	freeList = &proc[0];      // all procs are in freeList
	proc[NPROC-1].next = 0;
	readyQueue = sleepList = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);
	p->status = RUNNING;
	p->ppid   = 0;
	p->parent = p;
	running = p;
	nproc = 1;
	printf("done\n");
} 

int scheduler()
{
	// TODO: go to lab4.help to find the real way to handle running, etc.
	// the  p0 proc should be in the readyqueue, and it should have a status of READY
	// before it had a bug
	if (running->status == READY)
	{
		printf("enqueuing P%d\n", running->pid);
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	running->status = RUNNING;
	color = running->pid + 0x0A;
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
		// printSleepQueue(sleepList);
		printSleepQueue(sleepList);
		printf("-----------------------------------------\n");

		printf("proc %d[%d] running: parent=%d\n",
			running->pid, running->priority, running->ppid);

		printf("type a command [s|f|q|w| u ] : ");
		c = getc();
		printf("%c\n", c);

		switch(c){
			case 's' : do_tswitch();   break;
			case 'f' : do_kfork();     break;
			case 'q' : do_exit();      break;
			case 'w' : do_wait();      break;
			case 'u' :
				// printf("Running->uss: %x\n", running->uss);
				// printf("Running->usp: %x\n", running->usp);
				goUmode();
				break;   // let process goUmode
		}
	}
}

int int80h();
int set_vector(u16 segment, u16 handler)
{
	// put_word(word, segment, offset) in mtxlib
	put_word(handler, 0x0000, segment<<2);
	put_word(0x1000,  0x0000, (segment<<2) + 2);
}

u8 get_byte(u16 segment, u16 offset)
{
	u8 byte;
	setds(segment);
	byte = *(u8 *)offset;
	setds(MTXSEG);
	return byte;
}

// WRITE YOUR OWN get_word, which gets a WORD
int get_word(u16 segment, u16 offset)
{
	u16 word;
	setds(segment);
	word = *(u16 *)offset;
	setds(MTXSEG);
	return word;

	// // bit shift high << 8
	// u8 high, low;
	// setds(segment);
	// high = *(u8 *)(offset >> 8);
	// low = *(u8 *)(offset);
	// setds(MTXSEG);
	// return (high << 8) + low;
}

int put_byte(u8 byte, u16 segment, u16 offset)
{
	setds(segment);
	*(u8 *)offset = byte;
	setds(MTXSEG);
}

// WRITE YOUR OWN put_word, which writes a WORD
int put_word(u16 word, u16 segment, u16 offset)
{
	// Usage:
	// put_word(4, p->uss, p->usp+(2*9));
	// ** It's always times 2 of the amount of indexes.
	setds(segment);
	*(u16 *)offset = word;
	setds(MTXSEG);

	
}


main()
{
	printf("MTX starts in main()\n");
	init();      // initialize and create P0 as running
	set_vector(80, int80h);

	kfork("/bin/u1");     // P0 kfork() P1

	while(1)
	{
		printf("P0 running\n");
		while(!readyQueue);
		printf("P0 switch process\n");
		running->status = READY;
		tswitch();         // P0 switch to run P1
	}
}
