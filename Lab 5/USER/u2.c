#include "ucode.c"


int color;

main()
{ 
	char name[64]; int pid, cmd;

	while(1)
	{
		pid = getpid();

		color = 0x0C;

		printf("----------------------------------------------\n");
		printf("YOOOOOO I AM PROC%d IN UMODE!!!: MY RUNNING SEGMENT=%x\n", getpid(), getcs());
		show_menu();
		printf("WHAT IS YOUR COMMAND ? ");
		gets(name); 
		if (name[0]==0) 
		{
			continue;
		}

		cmd = find_cmd(name);
		switch(cmd)
		{
			case 0 : getpid();   break;
			case 1 : ps();       break;
			case 2 : chname();   break;
			case 3 : kfork();    break;
			case 4 : kswitch();  break;
			case 5 : wait();     break;
			case 6 : exit();	 break;
			case 7 : exec();     break;

			default: invalid(name); break;
		}
	}
}