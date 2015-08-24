
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

int get_word(u16 segment, u16 offset)
{
	u16 word;
	setds(segment);
	word = *(u16 *)offset;
	setds(MTXSEG);
	return word;
}

int put_byte(u8 byte, u16 segment, u16 offset)
{
	setds(segment);
	*(u8 *)offset = byte;
	setds(MTXSEG);
}

int put_word(u16 word, u16 segment, u16 offset)
{
	// Usage:
	// put_word(4, p->uss, p->usp+(2*9));
	// ** It's always times 2 of the amount of indexes.
	setds(segment);
	*(u16 *)offset = word;
	setds(MTXSEG);	
}

void printQueue(PROC *queue)
{
	// print the queue entries in [pid, priority]->  format;
	PROC *temp = queue;
	while(temp != 0)
	{
		printf("[%u, %u] -> ", temp->pid, temp->priority);
		temp = temp->next;
	}
	printf("[NULL]\n");
}

void printSleepQueue(PROC *queue)
{
	// print the sleep queue entries in [pid, priority]->  format;
	PROC *temp = queue;
	while(temp != 0)
	{
		printf("%u [event = %x] -> ", temp->pid, temp->event);
		temp = temp->next;
	}
	printf("[NULL]\n");
}

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
	PROC *prev = 0, *curr = 0;
	if(p == 0)
	{
		printf("input p is null.\n");
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