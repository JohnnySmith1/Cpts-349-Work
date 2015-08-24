#include "ucode.c"

#define BUF_SIZE 256
#define BLKSIZE 1024

main(int argc, char *argv[])
{
	int entryType;
	char *srcPath, *destPath;
	STAT srcStatBuf, destStatBuf;


	if(argc < 3)
	{
		printf("Hey! Use copy command like: cp [src] [dest]\n");
		return;
	}

	srcPath = argv[1];
	destPath = argv[2];

	stat(srcPath, &srcStatBuf);
	stat(destPath, &destStatBuf);

	// Check for copying a directory or not
	// 0x41ED
	if(srcStatBuf.st_mode == 0x41ED)
	{
		copyDir(srcPath, destPath);
	}
	else
	{
		copyFile(srcPath, destPath);
	}
	
}

int copyDir(char *src, char *dest)
{
	int sourceDev, destDev, n;
	char buf[BLKSIZE], tempName[256];
	char *cp;
	DIR *dp;
	STAT tempStatBuf, destStatBuf;

	// We have to recursively copy over a directory

	stat(dest, &destStatBuf);
	if(destStatBuf.st_ino != 0)
	{
		printf("Dest dir already exists!\n");
		return -1;
	}
	
	// We assume the destination directory doesn't exist
	// We create a directory
	printf("Making directory: %s\n", dest);
	mkdir(dest);

	// Open the src directory for read
	sourceDev = open(src, 0);
	
	n = read(sourceDev, buf, 1024);
	cp = buf;
	dp = (DIR *)buf;

	while(cp < (buf + BLKSIZE))
	{
		int pid, child, status;
		char srcDirEntryPath[BUF_SIZE], destDirEntryPath[BUF_SIZE];

		strcpy(tempName, dp->name);
		tempName[dp->name_len] = 0;

		if(strcmp(tempName, ".") == 0 || strcmp(tempName, "..") == 0)
		{
			cp += dp->rec_len;
			dp = (DIR *)cp;
			continue;
		}

		

		// Build the src path
		strcpy(srcDirEntryPath, src);
		strcat(srcDirEntryPath, "/");
		strcat(srcDirEntryPath, tempName);

		stat(srcDirEntryPath, &tempStatBuf);

		// Build the dest path
		strcpy(destDirEntryPath, dest);
		strcat(destDirEntryPath, "/");
		strcat(destDirEntryPath, tempName);



		if(tempStatBuf.st_mode == 0x41ED)
		{
			copyDir(srcDirEntryPath, destDirEntryPath);
		}
		else
		{
			copyFile(srcDirEntryPath, destDirEntryPath);
		}

		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
}


int copyFile(char *src, char *dest)
{
	// We are only copying over a single file
	int sourceDev, destDev;
    char readBuf[BUF_SIZE];
    int n;

    // sourceDev is the fd we use for the source pathname
    // We open src for reading
    sourceDev = open(src, 0);
    if(sourceDev == -1)
    {
        // If we get an error, we already printed the error, so we just return
        return;
    }

    printf("Copying file: %s...\n", dest);
    creat(dest);
    
    destDev = open(dest, 1);

    if(destDev == -1)
    {
        // we got an error, so we return
        printf("Error when opening dest file!\n");
        return;
    }
	
	while( (n = read(sourceDev, readBuf, BUF_SIZE)) )
    {
        // We only write n bytes to the file, so we keep track of it's exact size
        write(destDev, readBuf, n);
    }

    // printf("sourceDev: %d\n", sourceDev);
    // printf("destDev: %d\n", destDev);
    close(sourceDev);
    close(destDev);

}