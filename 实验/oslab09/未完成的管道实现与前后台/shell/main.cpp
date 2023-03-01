#include <iostream>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <fcntl.h>
using namespace std;

//全局常量定义
#define SUCCESS 0

//全局变量定义

int re_flag = 0;
char current_dir[100];
char user_dir[100];

string cmdline; //command input
string separator=";";
string command;


vector<string> commands; // 所有命令
vector<string> child_commands; //仅一个命令

typedef void (CMD_HANDLER)(void);
typedef struct builtin_cmd
{
    char* name;
    CMD_HANDLER *handler;
} BUILTIN_CMD;

//全局函数声明
void init();
int execute_command();
void read_command();
void parse_command();
int builtin(void);
void do_cd();
void do_ls();

//内部命令解析
BUILTIN_CMD builtins[] = {
        {(char*)"cd", do_cd},
        {(char*)"ls", do_ls},
        {NULL, NULL}

};



int main()
{

    printf("\033[32m**************welcome to mini shell******2013747****** \n\033[0m");
    strcpy(current_dir, getcwd(NULL, 0));
    strcpy(user_dir, getcwd(NULL, 0));
//    printf("PATH : %s\n", getenv("PATH"));
    while (1)
    {
        printf("\033[92m%s@MINISHELL\033[0m:\033[34m%s\033[0m$", getlogin(), current_dir);
        init();
        read_command();
        parse_command();

    }
    return 0;
}

void init(){
    for (int i = 0; i < child_commands.size(); i++)
    {
        child_commands[i].clear();
    }
    for (int i = 0; i < commands.size(); i++)
    {
        commands[i].clear();
    }
    cmdline.clear();
    command.clear();
    child_commands.clear();
    commands.clear();
}

void read_command()
{
    //处理cmdline
    getline(cin, cmdline); // input string with ' '
    typedef string::size_type string_size;
    string_size i = 0;
    while (i != cmdline.size())
    {
        int flag = 0;
        while (i != cmdline.size() && flag == 0)
        {
            flag = 1;
            for (string_size x = 0; x < separator.size(); ++x)
                if (cmdline[i] == separator[x])
                {
                    ++i;
                    flag = 0;
                    break;
                }
        }
        flag = 0;
        string_size j = i;
        while (j != cmdline.size() && flag == 0)
        {
            for (string_size x = 0; x < separator.size(); ++x)
                if (cmdline[j] == separator[x])
                {
                    flag = 1;
                    break;
                }
            if (flag == 0)
                ++j;
        }
        if (i != j)
        {
            commands.push_back(cmdline.substr(i, j - i));
            i = j;
        }
    }
}

void parse_command()
{
    //处理cmds
    for (int i = 0; i < commands.size(); i++)
    {
        child_commands.clear();
        command.clear();
      du  stringstream input2(commands[i]); // string stream initialize 不按照空格划分
        while (input2 >> command)
        {
            child_commands.push_back(command);
        }
        if (command == "exit")
        {
            printf("\033[32m****************** mini shell exit*******2013747******\n\033[0m");
            exit(0);
        }
        if (child_commands.size())
        {
            execute_command();
        }
    }
}

//内部命令解析
//执行返回1，没有表示0
int builtin(void){
    int i = 0;
    int found = 0;
    while(builtins[i].name != NULL){
        if(builtins[i].name == child_commands[0]) {
            builtins[i].handler();
            found=1;
            break;
        }
        i++;
    }
    return found;
}

int execute_command()
{

    if(builtin())
        return SUCCESS;
    else
    {
        pid_t pid;
        pid = fork();
        int status;
        const char *rest = child_commands[0].c_str();
        if (pid == 0)
        {
            //子线程
            char **cmd_temp = new char *[child_commands.size()];
            for (int i = 0; i < child_commands.size(); i++)
            {
                cmd_temp[i] = new char[500];
                memset(cmd_temp[i], 0, sizeof(*cmd_temp[i]));
            }
            for (int i = 0; i < child_commands.size(); i++)
            {
                strcpy(cmd_temp[i], child_commands[i].c_str());
            }
            cmd_temp[child_commands.size()] = NULL;
            if (execvp(rest, cmd_temp) < 0)
            {
                printf("\033[31m%s:command not found.\n\033[0m", child_commands[0].c_str());
            }

        }
        else if (pid > 0)
        {
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return SUCCESS;
}

void do_cd(){

        // 当前系统目录
        // char target_path[100];
        // getcwd(target_path, 100);
        // cout<<"now path"<<target_path<<endl;
        if (child_commands.size() == 1)
        {
            strcpy(current_dir, user_dir);
            chdir(user_dir);
        }
        else
        {
            const char *rest = child_commands[1].c_str();
            if (child_commands[1] == "/")
            {
                opendir(rest);
                strcpy(current_dir, rest);
                chdir(rest);

            }
            else if (child_commands[1] == "..")
            {
                char *parent_dir = dirname(current_dir);
                strcpy(current_dir, parent_dir);
                chdir(parent_dir);
            }
            else if (child_commands[1] == "~")
            {
                strcpy(current_dir, user_dir);
                chdir(user_dir);
            }
            else
            {
                char target_path[1024];
                cout << "current dit:" << current_dir << endl;
                if (strcmp(current_dir, "/") == 0)
                {
                    snprintf(target_path, 1024, "%s%s", current_dir, rest);
                }
                else
                {
                    snprintf(target_path, 1024, "%s/%s", current_dir, rest);
                }
                if (opendir(target_path) == NULL)
                {
                    cout << "cd: " << rest << ":";
                    printf("\033[31m没有那个文件或目录.\n\033[0m");
                }
                strcpy(current_dir, target_path);
                chdir(current_dir);
            }
            cout << current_dir << endl;
        }
    }

void do_ls(){


        pid_t pid;
        pid= fork();
        int status;
        int count = 0;
        const char *rest = child_commands[0].c_str();
        if (pid == 0)
        {
            for (int i = 0; i < child_commands.size(); i++)
            {

                if (child_commands[i] == ">")
                {
                    re_flag = 1;
                    count = i;
                }
                if (child_commands[i] == ">>")
                {
                    re_flag = 2;
                    count = i;
                }
            }
            if (re_flag != 0)
            {
                char **cmd_temp = new char *[count];
                for (int i = 0; i < count; i++)
                {
                    cmd_temp[i] = new char[500];
                    memset(cmd_temp[i], 0, sizeof(*cmd_temp[i]));
                }
                for (int i = 0; i < count; i++)
                {
                    strcpy(cmd_temp[i], child_commands[i].c_str());
                }
                cmd_temp[count] = current_dir;
                cmd_temp[count + 1] = NULL;
                // 标准输出重定向，将原本要写入标准输出 1 的数据写入新文件(fd)中
                int fd = 1;
                if (re_flag == 1)
                    fd = open(child_commands[count + 1].c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0664);
                else if (re_flag == 2)
                    fd = open(child_commands[count + 1].c_str(), O_CREAT | O_WRONLY | O_APPEND, 0664);
                dup2(fd, 1);
                if (execvp(rest, cmd_temp) < 0)
                {
                    printf("\033[31m%s:command not found.\n\033[0m", child_commands[0].c_str());
                }
            }
            else
            {
                char **cmd_temp = new char *[child_commands.size() + 1];
                for (int i = 0; i < child_commands.size(); i++)
                {
                    cmd_temp[i] = new char[500];
                    memset(cmd_temp[i], 0, sizeof(*cmd_temp[i]));
                }
                for (int i = 0; i < child_commands.size(); i++)
                {
                    strcpy(cmd_temp[i], child_commands[i].c_str());
                }
                cmd_temp[child_commands.size()] = current_dir;
                cmd_temp[child_commands.size() + 1] = NULL;
                if (execvp(rest, cmd_temp) < 0)
                {
                    printf("\033[31m%s:command not found.\n\033[0m", child_commands[0].c_str());
                }
            }
        }

        else if (pid > 0)
        {
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }


}