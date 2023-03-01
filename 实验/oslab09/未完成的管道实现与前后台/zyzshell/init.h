#ifndef _INIT_H_
#define _INIT_H_
#include <stdio.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

//常量定义
#define MAXLINE 1024 // max text line length
#define MAXARG 20  // max no. of arguments to a command
#define PIPELINE 10 // max no. of commands in a pipeline
#define MAXNAME 100 //max length of a file name

//结构体定义
typedef struct command
{
    char *args[MAXARG + 1]; // command line arguments
    int infd; // stdin redirection, 0 is default
    int outfd; // stdout redirection, 1 is default

} COMMAND;

//全局变量声明
extern char cmdline[MAXLINE+1];
extern char avline[MAXLINE+1];
extern COMMAND cmd[PIPELINE];

extern char *lineptr;
extern char *avptr;
extern char infile[MAXNAME+1];
extern char outfile[MAXNAME+1];
extern int cmd_count;
extern int backgnd;
extern int append;
extern int lastpid;



void setup(void);
void init(void);

#endif // _INIT_H_