#define BASE_DEC 10
#define BASE_OCT 8
#define BASE_HEX 16

char *table = "0123456789ABCDEF";

// Forward declarations of IO functions
void myprintf(char *fmt, ...);
int prints(char *x);
int printu(u32 x);
int printd(int x);
int printo(u32 x);
int printx(u32 x);
int rpu(u32 x);
int rpo(u32 x);
int rpx(u32 x);



void myprintf(char *fmt, ...)
{
    int i = 0;
    char *cp;
    int *bp;
    int *ip;
    int retPC;
    
    cp = fmt;
    bp = (int *)getbp();
    ip = bp;
    retPC = *(ip++);
    // To make ip point to first argument, we have to iterate it by 2
    ip += 2;


    
    i = 0;
    // We loop through the string.
    while(fmt[i] != '\0')
    {
        if(fmt[i] == '\n')  // Case where character is a newline
        {
            putc('\n');
            putc('\r');
        }
        else if(fmt[i] == '%')  // User wants to print either a char, string, int, etc.
        {
            char *cPtr = 0;
            switch(fmt[++i])
            {
                case 's':
                    cPtr = (char *)*ip;
                    prints(cPtr);
                    break;
                case 'u':
                    printu(*ip);    
                    break;
                case 'd':
                    printd(*ip);
                    break;
                case 'o':
                    printo(*ip);
                    break;
                case 'x':
                    printx(*ip);
                    break;
                case 'c':
                    putc(*ip);
                    break;
                default:
                    break;
            }
            ip++;
        }
        else
        {
            putc(fmt[i]);    // Case where just a regular character
        }
        i++;
    }
    
    return;
}


int printu(u32 x)
{
    if (x==0)
        putc('0');
    else
        rpu(x);
}

int rpu(u32 x)
{
    char c;
    if (x){
        c = table[x % BASE_DEC];
        rpu(x / BASE_DEC);
        putc(c);
    } 
}

int printo(u32 x)
{
    if (x==0)
        putc('\0');
    else
        rpo(x);
}

int rpo(u32 x)
{
    char c;
    if (x){
        c = table[x % BASE_OCT];
        rpo(x / BASE_OCT);
        putc(c);
    } 
}

int printd(int x)
{
    if (x==0)
        putc('0');
    else
    {
        if(x < 0)
        {
            putc('-');
            // We change x to it's positive 
            x += (2 * x * -1);
        }
    
        
        rpu(x);
    }

}

int printx(u32 x)
{
    if (x==0)
        putc('\0');
    else
        rpx(x);
}

int rpx(u32 x)
{
    char c;
    if (x){
        c = table[x % BASE_HEX];
        rpx(x / BASE_HEX);
        putc(c);
    } 
}

int prints(char *x)
{
    int i = 0;
    if (x==0)
        putc('\0');
    else
    {
        for(i = 0; i < strlen(x); i++)
        {
            putc(x[i]);
            if(x[i] == '\n')
            {
                putc('\r');
            }
        }
    }
}