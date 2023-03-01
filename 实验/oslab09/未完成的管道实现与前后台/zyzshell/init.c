#include "init.h"
#include<signal.h>
#include<stdio.h>
#include<string.h>


void sigint_handler(int sig);
void setup(void)
{
    signal(SIGINT, sigint_handler);//注册信号处理函数
    signal(SIGQUIT, SIG_IGN);//忽略信号
}
//信号处理函数
void sigint_handler(int sig)
{
    printf("\n[zyzshell]$ ");
    fflush(stdout);
}

void init(void){
    memset(cmd,0,sizeof(cmd));//初始化命令结构体
    int i;
    for(i=0;i<PIPELINE;i++){
        cmd[i].infd=0;//默认输入为0
        cmd[i].outfd=1;//默认输出为1
    }
    memset(cmdline,0,sizeof(cmdline));//初始化命令行
    memset(avline,0,sizeof(avline));//初始化参数行
    lineptr = cmdline;//初始化命令行指针
    avptr = avline;//初始化参数行指针
    memset(infile,0,sizeof(infile));//初始化输入文件名
    memset(outfile,0,sizeof(outfile));//初始化输出文件名


    cmd_count = 0;//初始化命令数
    backgnd = 0;//初始化后台运行标志
    append = 0;//初始化追加标志
    lastpid = 0;//初始化最后一个进程的pid

}