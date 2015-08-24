#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
"Saturn", "Uranus", "Neptune" };
OFT  oft[NOFT];
PIPE pipe[NPIPE];

/**************************************************
bio.o, queue.o loader.o are in mtxlib
**************************************************/
#include "wait.c"
#include "int.c"
#include "forkexec.c"
#include "pipe.c"

int color;




/********* These are just here to satisfy the linker lol *********/
int do_ps()
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

int kmode() 
{}

#define NAMELEN 30
int chname(char *y) 
{
    // WRITE C CODE to change running's name string;
    char buf[64];
    char *cp = buf;
    int count = 0; 

    while (count < NAMELEN){
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

int do_exit() 
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

int do_wait() 
{
    int pid, status;
    pid = kwait(&status);
    // print pid and status;
    if(pid != 0)
    {
        printf("Wait pid: %d\n", pid);  
    }
}

int do_tswitch()
{
    running->status = READY;
    return tswitch();
}
/****************************************************************/








int init()
{
    PROC *p;
    int i, j;
    printf("init ....");
    for (i=0; i<NPROC; i++)
    {   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];

        // clear fd[ ] array of PROC
        for (j=0; j<NFD; j++)
        {
            p->fd[j] = 0;
        }
    
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    // initialize all OFT and PIPE structures    
    for (i=0; i<NOFT; i++)
    {
        oft[i].refCount = 0;
    }
    
    for (i=0; i<NPIPE; i++)
    {
        pipe[i].busy = 0;
    }
    

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = READY;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{
    if (running->status == READY)
    {
        enqueue(&readyQueue, running);
    }
    printList("readyQueue", readyQueue);
    running = dequeue(&readyQueue);
    running->status = RUNNING;
}

int int80h();

int set_vector(u16 vector, u16 addr)
{
    u16 location,cs;
    location = vector << 2;
    put_word(addr, 0, location);
    put_word(0x1000,0,location+2);
}

main()
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vector(80,int80h);

    printf("Got here\n");
    kfork("/bin/u1");     // P0 kfork() P1

    while(1)
    {
        printf("P0 running\n");
        if (nproc==2 && proc[1].status != READY)
        printf("no runable process, system halts\n");
        while(!readyQueue);
        printf("P0 switch process\n");
        tswitch();   // P0 switch to run P1
    }
}
