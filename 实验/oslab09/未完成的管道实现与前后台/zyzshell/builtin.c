#include <stdlib.h>
#include <stdio.h>
#include "init.h"
#include "builtin.h"
#include "parse.h"

typedef void (CMD_HANDLER)(void);
typedef struct builtin_cmd
{
    char *name;
    CMD_HANDLER *handler;
} BUILTIN_CMD;

void do_cd();
void do_exit(void);

BUILTIN_CMD builtins[] = {
    {"cd", do_cd},
    {"exit", do_exit},
    {NULL, NULL}

};
//内部命令解析
//执行返回1，没有表示0
int builtin(void){
    int i = 0;
    int found = 0;
    while(builtins[i].name != NULL){
        if(check(builtins[i].name)) {
            builtins[i].handler();
            found=1;
            break;
        }
        i++;
    }
    return found;

}

void do_cd(){
    printf("cd ...");
}
void do_exit(void){
    printf("exit");
    exit(EXIT_SUCCESS);
}