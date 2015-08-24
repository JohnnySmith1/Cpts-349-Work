
/*************************************************************************
usp  1   2   3   4   5   6   7   8   9  10   11    12    13  14  15  16
----------------------------------------------------------------------------
|uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
	u16 segment, offset;
	int a, b, c, d, r;

	segment = running->uss;
	offset = running->usp;



	// ==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
	// Gets these from the user mode
	a = get_word(segment, offset + 2*13);
	b = get_word(segment, offset + 2*14);
	c = get_word(segment, offset + 2*15);
	d = get_word(segment, offset + 2*16);

	switch(a) {
		case 0 : r = kgetpid();        break;
		case 1 : r = kps();            break;
		case 2 : 
			r = kchname(b);       
			break;
		case 3 : r = kkfork();         break;
		case 4 : r = ktswitch();       break;
		case 5 : 
			r = kkwait(b);        
			put_word(*(int *)b, segment, b);
			break;
		case 6 : r = kkexit(b);        break;
		case 7 : r = khop();           break;

		case 90: 
			r = getc();           
			break;
		case 91: 
			r = putc(b);
			break;
		case 99: kkexit(b);            break;
		default: printf("invalid syscall # : %d\n", a); 
	}

	// ==> WRITE CODE to let r be the return value to Umode
	// ktswitch() just calls tswitch
	// kkexit(b) doesn't ever return.
	// be sure to put r in the ax register
	put_word(r, segment, offset + 2*8);
}

//============= WRITE C CODE FOR syscall functions ======================
//==============return (int) value >= 0 for OK, or -1 for BAD. ==========
int kgetpid()
{
	// WRITE YOUR C code
	return running->pid;
}

int kps()
{
	// // WRITE C code to print PROC information
	return do_ps();
}

#define NAMELEN 32
u8 get_byte(u16 segment, u16 offset);
int kchname(char *y)
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

int kkfork()
{
	int result;
	PROC *p;
	// use you kfork() in kernel;
	// return child pid or -1 to Umode!!!
	p = kfork("/bin/u1");

	if(p)
	{
		result = p->pid;
	}
	else
	{
		result = -1;
	}
	return result;
}

int ktswitch()
{
	running->status = READY;
	return tswitch();
}

int kkwait(int *status)
{
	// use YOUR kwait() in LAB3;
	return kwait(status);
	

}

int kkexit(int value)
{
	// use your kexit() in LAB3
	// do NOT let P1 die
	kexit(value);
}

int khop()
{
	int segment, i;
	char segmentStr[4];
	PROC *temp;

	if(running->pid != 1)
	{
		printf("Only P1 can be casanova proc.\n");
		return;
	}

	printf("Enter segment you want to jump to (3-9): ");
	segment = atoi(gets(segmentStr));


	// // Check for proc at segment. If so, kill the proc
	// temp = readyQueue;
	// while(temp != 0)
	// {
	// 	if(temp->pid == (segment - 1))
	// 	{
	// 		printf("About to kill proc %u\n", temp->pid);
	// 		prockexit(123, temp->pid);
	// 		break;
	// 	}
	// 	temp = temp->next;
	// }


	

	segment = 0x1000*segment;

	printf("your segment is: %x\n", segment);

	// This line loads the file
	// (4).1    SETUP new PROC's ustack for it to return to Umode to execute filename;
	load("/bin/u1", segment);
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
	running->usp = -2 * 12;
	// Set new PROC.uss = segment;
	running->uss = segment;


}