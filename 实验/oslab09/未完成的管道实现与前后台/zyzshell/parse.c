#include "init.h"
#include "parse.h"
#include "builtin.h"
#include <stdio.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <sys/syslimits.h>
#include <sys/fcntl.h>

void get_command(int i);

void get_name(char *name);
void print_command();
void forkexec(COMMAND *pcmd);

/*
 * Function: shell-main-loop
 */
void shell_loop(void)
{
    while (1)
    {
        printf("[zyzshell]$ ");
        rewind(stdin); //清除缓冲区
        init();
        if (read_command() == -1)
        {
            break;
        }
        parse_command();
        print_command();
        execute_command();
    }
    printf("\nexit\n");
}

/*
 * Function: read command
 * success: return 0
 * fail or read EOF(文件结束符): return -1
 */
int read_command(void)
{

    if (fgets(cmdline, MAXLINE, stdin) == NULL)
        return -1;

    return 0;
}

/*
 * Function: parse command
 * success: return the number of command
 * fail: return -1
 */
int parse_command(void)
{
    //成功返回命令个数，失败返回-1
    if(check("\n"))
        return 0;

    if(builtin())
        return 0;



    //1. 解析第一条简单命令
    if(check("\n"))
        return 0;
    get_command(0);
    //2. 判定是否有输入重定向符
    if(check("<")){
        get_name(infile);
    }

    //3. 判定是否有管道
    int i;
    for(i=1;i<PIPELINE;i++){
        if(check("|")){
            get_command(i);
        } else {
            break;
        }
    }
    //4. 判定是否有输出重定向符
    if(check(">")){
        if(check(">")){
            append = 1;
        }
        get_name(outfile);
    }
    //5. 判定是否后台作业
    if(check("&")){
        backgnd = 1;
    }
    //6. 判定命令结束'\n'
    if(check("\n")){
        cmd_count = i;
        return cmd_count;
    }
    else{
        fprintf(stderr,"Command line syntax error\n");
        return -1;
    }

    return 0;
}

/*
 * Function: execute command
 * success: return 0
 * fail: return -1
 */
int execute_command(void)
{
    /* ls | grep a | wc -l */

    if(infile[0] != '\0'){
        cmd[0].infd = open(infile,O_RDONLY);
    }
    if(outfile[0] != '\0'){
        if(append){
            cmd[cmd_count-1].outfd = open(outfile,O_WRONLY|O_CREAT|O_APPEND,0666);
        } else {
            cmd[cmd_count-1].outfd = open(outfile,O_WRONLY|O_CREAT|O_TRUNC,0666);
        }
    }

    int i;
    int fd;
    int fds[2];
    for ( i = 0; i <cmd_count ; ++i) {
        //如果不是最后一条命令，要创建管道
        if(i<cmd_count-1){
            pipe(fds);
            cmd[i].outfd = fds[1];//管道的写端
            cmd[i+1].infd = fds[0];//管道的读端
        }

        forkexec(&cmd[i]);

        //关闭管道的读写端
        if((fd=cmd[i].infd)!=0){
            close(fd);
        }
        if((fd=cmd[i].outfd)!=1){
            close(fd);
        }
    }
//    while(wait(NULL) != lastpid);
    wait(NULL);



    return 0;
}

void print_command(){
    int i;
    int j;
    printf("cmd_count:%d\n",cmd_count);
    if(infile[0] != '\0'){
        printf("infile:%s\n",infile);
    }
    if(outfile[0] != '\0'){
        printf("outfile:%s\n",outfile);
    }
    for( i=0;i<cmd_count;i++){
         j=0;
        while(cmd[i].args[j] != NULL){
            printf("[%s]",cmd[i].args[j]);
            j++;
        }
        printf("\n");
    }

}

/*
 * Function: 解析简单命令
 * 提取命令参数到avline数组中
 * 并且将COMMAND结构中的args数组中的每个指针指向avline数组中的参数
 */
void get_command(int i){
    int j=0;
    int inword = 0;
    while(*lineptr != '\0'){
        //跳过空格
       while(*lineptr == ' ' || *lineptr == '\t'){
           lineptr++;
       }
       //将第i条命令的第j个参数指向avptr
       cmd[i].args[j] = avptr;
       //提取参数
         while(*lineptr != ' ' && *lineptr != '\t' && *lineptr != '\0'&& *lineptr != '\n' && *lineptr != '|' && *lineptr != '<' && *lineptr != '>' && *lineptr != '&'){
              //将参数复制到avline数组中
             *avptr++ = *lineptr++;
                inword = 1;
         }
            *avptr++ = '\0';
        switch (*lineptr) {
            case ' ':
            case '\t':
                inword = 0;
                j++;
                break;
            case '<':
            case '>':
            case '&':
            case '|':
            case '\n':
                if(inword == 0){
                    cmd[i].args[j] = NULL;
                }
                return;
            default://'\0'
                return;
        }
    }




}
/*
 * Function: 判断是否有特殊符号
 * 成功返回1，lineptr指向特殊符号后面的字符
 * 失败返回0，lineptr不变
 */
int check(const char *str){
    char *p;
    //跳过空格
    while(*lineptr == ' ' || *lineptr == '\t'){
        lineptr++;
    }
    p = lineptr;
    while(*str != '\0'&&*p == *str){
        p++;
        str++;

    }
    if(*str == '\0'){
        lineptr = p;//更新lineptr
        return 1;
    }

    //不匹配,linptr不变
    return 0;

}
void get_name(char *name){
    //跳过空格
    while(*lineptr == ' ' || *lineptr == '\t'){
        lineptr++;
    }
    while(*lineptr != ' '
        && *lineptr != '\t'
        && *lineptr != '\0'
        && *lineptr != '\n'
        && *lineptr != '|'
        && *lineptr != '<'
        && *lineptr != '>'
        && *lineptr != '&'){
        *name++ = *lineptr++;
    }
    *name = '\0';

}

void forkexec(COMMAND *pcmd){
    int i;
    pid_t pid;
    pid = fork();
    if(pid >0){
        //父进程
        lastpid = pid;

    }
    else if(pid == 0){
        if(pcmd->infd == 0 && pcmd->outfd == 1){
            dup(pcmd->infd);
        }
        //子进程
        if(pcmd->infd != 0){
            close(0);
            dup(pcmd->infd);
        }
        if(pcmd->outfd != 1){
            close(1);
            dup(pcmd->outfd);
        }
        //关闭3以上的文件描述符
        for(i=3;i<OPEN_MAX;++i){
            close(i);
        }
        //执行命令
        execvp(pcmd->args[0],pcmd->args);
        //如果执行到这里，说明命令执行失败
        fprintf(stderr,"Command not found:%s\n",pcmd->args[0]);
        exit(EXIT_FAILURE);

    }
    else{
        fprintf(stderr,"fork error\n");
        exit(1);
    }
}