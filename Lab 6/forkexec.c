extern PROC *kfork(char *filename);

int copyImage(u16 child_segment)
{
  // copies the Umode image of running to child_segment.
  // copy_image() implies that the Umode images of parent and child are
  //     IDENTICAL. So, their saved usp must also be the same value (each is an 
  //     offset relative to its own segment).
  u16 i, runningSegment;
  u16 tempWord;

  runningSegment = running->uss;

  for(i = 0; i < (32*1024); i++)
  {
    tempWord = get_word(runningSegment, i*2);
    put_word(tempWord, (u16)child_segment, i*2);
  }
}

int fork()
{
  int i, pid;  
  u16 segment;
  PROC *p;

  
  p = (PROC *)kfork(0);   // kfork() but do NOT load any Umode image for child 
  if (p == 0)
  {
    // kfork failed 
    return -1;
  }
  


  segment = (p->pid+1)*0x1000;
  copyImage(segment);

  // YOUR fork() function
  // Set new PROC.uss = segment;
  p->uss = segment;
  // usp is the only thing that is copied over from parent
  p->usp = running->usp;

  // uDS 
  put_word(segment, segment, p->usp);
  // uES,
  put_word(segment, segment, p->usp+(2*1));
  // and uCS
  put_word(segment, segment, p->usp+(2*10));
  // the flag
  put_word(0x0200, segment, p->usp+(2*11));


  /**** Copy file descriptors ****/
  for (i=0; i<NFD; i++)
  {
    p->fd[i] = running->fd[i];
    if (p->fd[i] != 0)
    {
      p->fd[i]->refCount++;
      if (p->fd[i]->mode == READ_PIPE)
      {
        p->fd[i]->pipe_ptr->nreader++;
      }
      
      if (p->fd[i]->mode == WRITE_PIPE)
      {
        p->fd[i]->pipe_ptr->nwriter++;
      }
      
    }
  }

  return(p->pid);
}

int exec(char *filename)
{
  int segment, i;



  if(strcmp(filename, "/bin/u1") != 0 && strcmp(filename, "/bin/u2") != 0)
  {
    printf("File not found.\n");
    return -1;
  }

  segment = 0x1000*(running->pid+1);

  // We load the new file
  load(filename, segment);

  // We now fix the stack
  //  (LOW)  uSP                                | by INT 80  |   HIGH
 //     ---------------------------------------------------------------------
 //           |uDS|uES| di| si| bp| dx| cx| bx| ax|uPC|uCS|flag| XXXXXX
 //     ---------------------------------------------------------------------
 //            -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1 | 0 

 //        (a). re-establish ustack to the very high end of the segment.
 //        (b). "pretend" it had done  INT 80  from (virtual address) 0: 
 //             (c). fill in uCS, uDS, uES in ustack
 //             (d). execute from VA=0 ==> uPC=0, uFLAG=0x0200, 
 //                                        all other registers = 0;
 //             (e). fix proc.uSP to point at the current ustack TOP (-24)

 //     Finally, return from exec() ==> goUmode().

  // Reset all registers to 0.
  for(i = 1; i <= 12; i++)
  {
    put_word(0, segment, 2*-i);
  }

  // uCS
  put_word(segment, segment, 2*-2);
  // uDS
  put_word(segment, segment, 2*-12);
  // uES
  put_word(segment, segment, 2*-11);
  // uPC
  put_word(0, segment, 2*-3);
  // uFLAG
  put_word(0x0200, segment, 2*-1);
  // Correct usp to point to top of stack
  running->usp = 2*-12;

  return goUmode();
}

