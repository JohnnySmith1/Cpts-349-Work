#include "ucode.c"

int main(int argc, char *argv[])
{
	int i, fd;
	char buf[100];
	char *file = argv[1];


	for(i = 0; i < 4; i++)
	{
		gets(buf);
		printf("You entered: %s\n", buf);
	}



	return 0;
}