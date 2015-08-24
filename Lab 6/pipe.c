#define NAMELEN 30

void show_pipe(PIPE *p)
{
  int i, j;
  printf("--------------- PIPE CONTENTS ------------\n");     
    // print pipe information
  printf("Address: %x\n", p);
  printf("Buf: ");

  for(i = 0; i < p->data; i++)
  {
    printf("%c", p->buf[(p->head+i) % PSIZE]);
  }
  printf("\n");

  printf("Head: %d \t Tail: %d\n", p->head, p->tail);
  printf("Data: %d \t Room: %d\n", p->data, p->room);
  printf("nreader: %d \t nwriter: %d\n", p->nreader, p->nwriter);
  printf("busy: %d\n", p->busy);
  printf("\n----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE","WRITE_PIPE"};

int pfd()
{
  int i = 0;
  // print running process' opened file descriptors
  printf("fd     refCount    mode\n");
  printf("--     --------    -----\n");

  for(i = 0; i < NFD; i++)
  {
    if(running->fd[i] != 0)
    {
      printf("%d      ", i);
      printf("%d          ", running->fd[i]->refCount);
      switch(running->fd[i]->mode)
      {
        case READ_PIPE:
        printf("READ\n");
        break;
        case WRITE_PIPE:
        printf("WRITE\n");
        break;
        default: 
        printf("Mode type unrecognized.\n");
        break;
      }
    }
    
  }
}

OFT *FindOpenOFT()
{
  int i;
  OFT *temp;

  temp = 0;

  for(i = 0; i < NOFT; i++)
  {
    if(oft[i].refCount == 0)
    {
      temp = &oft[i];
      oft[i].refCount = 1;
      break;
    }
  }

  return temp;
}

int write_pipe(int fd, char *y, int n)
{
  int count, totalWritten, bufIndex;
  char buf[64];
  char *cp;
  OFT *WriteOftPtr;
  PIPE *RunningPipe;

  // WRITE C CODE to change running's name string;
  cp = buf;
  count = 0; 


  while (count < NAMELEN){
    *cp = get_byte(running->uss, y);
    if (*cp == 0) break;
    cp++; 
    y++; 
    count++;
  }
  buf[count] = 0;

  printf("Buf to write: %s\n", buf);
   

  WriteOftPtr = running->fd[fd];
  RunningPipe = WriteOftPtr->pipe_ptr;

  // your code for write_pipe()
  //          n = write(pd[1], buf, nbytes);

  // which tries to write nbytes of data to the pipe, subject to these 
  // constraints.
  //      ------------------------------------------------------------------
  // (1). If (no READER on the pipe) return BROKEN_PIPE_ERROR;
  //      ------------------------------------------------------------------
  //                 (pipe still have READERs):
  // (2). If (pipe has room){
  //          write as much as it can until all nbytes are written or (3).
  //          "wakeup" READERs that are waiting for data.  
  //      }
  // (3)  If (no room in pipe){
  //         "wakeup" READERs that are waiting for data
  //         "wait" for room; 
  //          then try to write again from (1).
  //      }

  // Show pipe information BEFORE


  printf("Running pipe BEFORE:\n");
  show_pipe(RunningPipe);

  

  if(RunningPipe->nreader <= 0)
  {
    printf("BROKEN_PIPE_ERROR \n");
    return -1;
  }


  bufIndex = 0;
  totalWritten = 0;


  // In case we have to iterate again because initially there wasn't enough 
  // room in the pipe, so we have to sleep()
  while(totalWritten < n)
  {
    if(RunningPipe->room > 0)
    {
      // We mod by PSIZE to "reset" the tail back to the beginning
      // because the pipe is a "loop"
      while(RunningPipe->room > 0 && totalWritten < n)
      {
        printf("Writing '%c' at tail=%u\n", buf[bufIndex], RunningPipe->tail);
        RunningPipe->buf[RunningPipe->tail] = buf[bufIndex];
        RunningPipe->tail = (RunningPipe->tail + 1) % PSIZE;

        RunningPipe->room--;
        RunningPipe->data++;
        bufIndex++;
        totalWritten++;
      }
      
      printf("Total written in write_pipe so far: %u\n", totalWritten);
    }
    else
    {
      // There is no room so we have to wait
      
      // Wakeup any readers that are sleeping on pipe
      printf("Waking up readers sleeping on %x!!\n", &RunningPipe->data);
      gets();
      kwakeup(&RunningPipe->data);

      printf("Writer sleeping on %x!!!\n", &RunningPipe->room);
      gets();
      kwait(&RunningPipe->room);
      printf("writer has returned!!!\n");
    }


  }

  printf("Running pipe AFTER:\n");
  show_pipe(RunningPipe);


  // printf("PAUSE\n");
  // getc();
  return totalWritten;
}

int read_pipe(int fd, char *y, int n)
{
  int count, numRead, index;
  char buf[64];
  char cp, *userModeBuf;
  OFT *ReadOftPtr;
  PIPE *RunningPipe;

  ReadOftPtr = running->fd[fd];
  RunningPipe = ReadOftPtr->pipe_ptr;

  printf("Pipe BEFORE: \n");
  show_pipe(RunningPipe);

  // your code for read_pipe()
  // n = read(pd[0], buf, nbytes);

  //    which tries to read nbytes from the pipe, subject to these constraints:
  //    ---------------------------------------------------------------------
  //    (1). If (no WRITER on the pipe){
  //            read as much as it can; either nbytes or until no more data.
  //            return ACTUAL number of bytes read
  //         }.
  //         --------------------------------------------------------------
  //                    (pipe still have WRITERs)
  //    (2). if (pipe has data){
  //            read until nbytes or (3).
  //            "wakeup" WRITERs that are waiting for room
  //         }
  //    (3). if (NO data in pipe){
  //            "wakeup" WRITERs that are waiting for room
  //            "wait" for data; 
  //            then try to read again from (1).
  //         }

  // Case where there is no writer on the pipe
  numRead = 0;
  index = 0;


  
  while(numRead < n)
  {

    if(RunningPipe->nwriter <= 0)
    {
      while(RunningPipe->data > 0 && numRead < n)
      {
        cp = RunningPipe->buf[RunningPipe->head];
        printf("Putting %c to userMode\n", cp);
        put_byte(cp, running->uss, (y + index));
        RunningPipe->head = (RunningPipe->head + 1) % PSIZE;
        RunningPipe->room++;
        RunningPipe->data--;
        index++;
        numRead++;
      }
      
      return numRead;
    }


  

    // Case where writer is on pipe, and the pipe has data
    if(RunningPipe->data > 0)
    {
      while(RunningPipe->data > 0 && numRead < n)
      {
        cp = RunningPipe->buf[RunningPipe->head];
        printf("Reading %c to userMode\n", cp);
        put_byte(cp, running->uss, (y + index));
        RunningPipe->head = (RunningPipe->head + 1) % PSIZE;
        RunningPipe->room++;
        RunningPipe->data--;
        index++;
        numRead++;
      }
      


      
    }
    else // If pipe has NO data
    {
      // We "wakeup" the writers that are waiting for room
      printf("Waking up writers sleeping on %x!!", &RunningPipe->room);
      gets();
      kwakeup(&RunningPipe->room);

      // We "wait" for data
      printf("reader sleeping on %x!!!\n", &RunningPipe->data);
      gets();
      kwait(&RunningPipe->data);
    }
  }


  printf("Pipe AFTER: \n");
  show_pipe(RunningPipe);

  return numRead;

}

int kpipe(int pd[2])
{
  int i;
  int readIndex, writeIndex;
  PIPE *NewPipe;
  char temp[10];

  NewPipe = 0;
  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors


  // Find the OFT for reading
  for(i = 0; i < NFD; i++)
  {
    if(running->fd[i] == 0)
    {
      printf("Found open OFT for READ at fd[%u]\n", i);
      pd[0] = i;
      readIndex = i;
      running->fd[i] = FindOpenOFT();
      break;
    }
  }

  // Find the OFT for writing
  for(i = 0; i < NFD; i++)
  {
    if(running->fd[i] == 0)
    {
      printf("Found open OFT for WRITE at fd[%u]\n", i);
      pd[1] = i;
      writeIndex = i;
      running->fd[i] = FindOpenOFT();
      break;
    }
  }


  // printf("DOUBLE PAUSED\n");
  // gets();

  // We search through pipe array to find a free pipe struct
  for(i = 0; i < NPIPE; i++)
  {
    if(pipe[i].busy == 0)
    {
      printf("New pipe found at pipe[%u]\n", i);
      NewPipe = &pipe[i];
      break;
    }
  }

  if(NewPipe == 0)
  {
    printf("Error: Too many pipes!!\n");
    return -1;
  }

  

  // Make the new pipe busy
  NewPipe->busy = 1;
  // Initialize everything else
  for(i = 0; i < PSIZE; i++)
  {
    NewPipe->buf[i] = 0;
  }
  NewPipe->head = 0;
  NewPipe->tail = 0;
  NewPipe->nreader = 1;
  NewPipe->nwriter = 1;
  NewPipe->data = 0;
  NewPipe->room = PSIZE;


  

  // Set running proc's fd pointers
  // For READ fd
  running->fd[readIndex]->mode = READ_PIPE;
  running->fd[readIndex]->refCount = 1;
  running->fd[readIndex]->pipe_ptr = NewPipe;
  // For WRITE fd
  running->fd[writeIndex]->mode = WRITE_PIPE;
  running->fd[writeIndex]->refCount = 1;
  running->fd[writeIndex]->pipe_ptr = NewPipe;



  // printf("PAUSE\n");
  // getc();


  
  return 0;


}



int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry 

  if (op->mode == READ_PIPE)
  {
    pp = op->pipe_ptr;
    pp->nreader--;                   // dec n reader by 1

    if (--op->refCount == 0)
    {        // last reader
      if (pp->nwriter <= 0)
      {         // no more writers
        pp->busy = FREE;             // free the pipe   
        return;
      }
    }
    kwakeup(&pp->room);               // wakeup any WRITER on pipe 
    return;
  }
  else if(op->mode == WRITE_PIPE)
  {
    // YOUR CODE for the WRITE_PIPE case:
    pp = op->pipe_ptr;
    pp->nwriter--;                   // dec n writer by 1

    if (--op->refCount == 0)
    {        // last writer
      if (pp->nreader <= 0)
      {         // no more readers
        pp->busy = FREE;             // free the pipe   
        return;
      }
    }
    kwakeup(&pp->room);               // wakeup any READER on pipe 
    return;
  }

  

}
