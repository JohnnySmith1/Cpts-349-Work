
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
	int a, b, c, d, r = 0;
	int segmentNum, segmentStr;
	char myName[10];

	segment = running->uss;
	offset = running->usp;



	// ==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
	// Gets these from the user mode
	// PA
	a = get_word(segment, offset + 2*9);
	b = get_word(segment, offset + 2*10);
	c = get_word(segment, offset + 2*11);
	d = get_word(segment, offset + 2*12);

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
		case 12 :
			printf("GETTING MY NAME\n");
			r = getMyName(b);
			printf("r is: %u\n", r);
			// put_word(r, segment, offset + 2*4);
			break;

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
	put_word(r, segment, offset + 2*4);
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

int getMyName(char myname[64])
{
	// WRITE C CODE to change running's name string;
	int length = 0;
	int index = 0;
	char cp;


	printf("running->name: %s\n", running->name);

	
	while(index < NAMELEN)
	{
		cp = running->name[index];
		printf("putting cp: %c\n", cp);
		put_byte(cp, running->uss, (myname + index));

		
		if(cp == 0)
			break;
		// myname[index] = cp;

		length++;
		index++;
	}

	myname[index] = 0;


	printf("length: %u\n", length);




	// char buf[64];
	// char *cp = buf;
	// int count = 0; 

	// while (count < NAMELEN){
	// 	*cp = get_byte(running->uss, y);
	// 	if (*cp == 0) break;
	// 	cp++; 
	// 	y++; 
	// 	count++;
	// }
	// buf[31] = 0;

	// printf("changing name of proc %d to %s\n", running->pid, buf);
	// strcpy(running->name, buf); 
	// printf("done\n");







	return length;
	



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