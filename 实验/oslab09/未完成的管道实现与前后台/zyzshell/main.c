#include "init.h"
#include "parse.h"

//全局变量定义
char cmdline[MAXLINE+1];
char avline[MAXLINE+1];
char *lineptr;
char *avptr;
char infile[MAXNAME+1];
char outfile[MAXNAME+1];
COMMAND cmd[PIPELINE];
int cmd_count;
int backgnd;
int append;
int lastpid;

int main()
{
    setup();
    shell_loop();
    return 0;
}

