/************** t.c file **************************/
int prints(char *s)
{
  // call putc(c) to print the string s;
  char *temp = s;
  while(*temp != '\0')
  {
    putc(*temp);
    temp++;
  }
  return 1;
}

int gets(char s[ ])
{
  // call getc() to input a string into s[ ]
  int index = 0;
  char input;
  input = getc();
  while(input != '\r')
  {
    s[index] = input;
    putc(s[index]);
    index++;

    input = getc();
  }
  s[index] = '\0';


  return 1;
}

main()
{
  char name[64];
  while(1)
  {
    prints("What's your name? ");
    gets(name);
    if (name[0]==0)
      break;
    prints("\n\rWelcome ");
    prints(name); 
    prints("\n\r");
  }
  prints("return to assembly and hang\n\r");
  return;





}