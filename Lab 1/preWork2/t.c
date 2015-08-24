/********** t.c file ******************/

#include "ext2.h"


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

u16 getblk(u16 blk, char buf[ ])
{
	int cyl, head, sector;

	// convert blk to CHS; 
	cyl = blk / 18;
	head = blk / 9;
	head = head % 2;
	sector = blk % 9;
	sector = sector * 2;

	readfd(cyl, head, sector, buf);
}


GD    *gp;
INODE *ip;
DIR   *dp;

main()
{ 
	u16  i, iblk;
	char buf1[BLK], buf2[BLK], *cp;
	char name[20];

	prints("booter start\n\r");  

	/* read blk#2 to get group descriptor 0 */
	getblk((u16)2, buf1);
	gp = (GD *)buf1;
	iblk = (u16)gp->bg_inode_table; // typecast u32 to u16

	prints("inode_block=");
	putc(iblk+'0');
	prints("\r\n");
	getc();

	/******** write C code to do these: **************/
	// (1).read first inode block into buf1[ ]
	getblk((u16)iblk, buf1);
	
	// (2).let ip point to root INODE (inode #2)
	ip = (INODE *)buf1 + 1;
	
	
	// (3).For each DIRECT block of / do:
  	// read data block into buf2[ ];
  	// step through the data block to print the names 
	// of the dir entries 
	for(i = 0; i < 12; i++)
	{
		if(ip->i_block[i] == 0)
			break;

		getblk((u16)ip->i_block[i], buf2);
		cp = (char *)buf2;

		while(cp < (buf2 + BLK))
		{
			dp = (DIR *)cp;
			strncpy(name, "", 20);
			strncpy(name, dp->name, dp->name_len);
			prints(name);
			prints("\r\n");
			cp += dp->rec_len;
		}


	}

	prints("\n\rAll done\n\r\n\r");
	return;

}  