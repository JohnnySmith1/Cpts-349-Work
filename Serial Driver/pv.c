struct semaphore{
	int value;
	int  lock;
	PROC *queue;
};

//#define interrupts_off() { /* save    CPU.SR;  mask out interrupts; */}
//#define interrupts_on()  { /* restore CPU.SR to saved SR; */ }
//#define LOCK(x)      while(TS(x));
//#define UNLOCK(x)    x=0;

int P(struct semaphore *s)
{
	// this function is for waiting!

	// write YOUR C code for P()
	// interrupts_off();        /* see above macro #defines */
	// LOCK(s->lock);         /* keep trying on the spin lock */

	s->value--;
	if (s->value < 0)
	{
		ksleep(s->queue);   /* block caller;
	                       enqueue caller to s->queue);
	                       UNLOCK(s->lock);
	                       switch process;
	                      <================= resume point !!!!  */
	}
	// else
	// {
	//   UNLOCK(s->lock);
	// }
	// <==== /* resume point is here if waited in s-queue */ 
	// interrupts_on();          /* restore saved CPU status Reg */
	
}

int V(struct semaphore *s)
{
	// write YOUR C code for V()
	// interrupts_off();
	// LOCK(s->lock);

	s->value++;
	if (s->value <= 0)
	{
		kwakeup(s->queue); /* dequeue and wakeup
	                     first waiter from s->queue 
	                  */
	}
	// UNLOCK(s->lock);
	// interrupts_on(); 
	
}
