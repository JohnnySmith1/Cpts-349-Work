typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5
#define MTXSEG 0x1000

typedef struct proc
{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];

    int    kstack[SSIZE];      // per proc stack area
} PROC;


// // t.c
// int init();
// int scheduler();
// int body();
// int set_vector(u16 segment, u16 handler);
// u8 get_byte(u16 segment, u16 offset);
// int get_word(u16 segment, u16 offset);
// int put_byte(u8 byte, u16 segment, u16 offset);
// int put_word(u16 word, u16 segment, u16 offset);


// // queue.c
// void do_kfork();
// void do_tswitch();
// void do_ps();
// void do_wakeup();
// void do_sleep();
// void do_wait();
// void do_exit();
// void printQueue(PROC *queue);
// void printSleepQueue(PROC *queue);
// PROC *dequeue(PROC **queue);
// PROC *get_proc();
// void put_proc(PROC *p);
// void enqueue(PROC **queue, PROC *p);


// // kernel.c
// int ksleep(int event);
// int kwakeup(int event);
// int kexit(int exitvalue);
// int kwait(int *status);


// // fork.c
// PROC *kfork(char *filename);
// int fork();
// int copy_image(u32 child_segment);


// // int.c
// int kcinth();
// int kgetpid();
// int kps();
// int kchname(char *y);
// int kkfork();
// int ktswitch();
// int kkwait(int *status);
// int kkexit(int value);