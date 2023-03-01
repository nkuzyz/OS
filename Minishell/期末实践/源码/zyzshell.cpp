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
#include <cstdlib>
using namespace std;

//全局常量定义
#define SUCCESS 0
#define BUFFSIZE 100
//全局变量定义

char current_dir[BUFFSIZE]; //当前所在的系统路径
char user_dir[BUFFSIZE];  //当前所在的系统路径
uid_t uid; //获得用户id
string cmdline; //从终端读入的字符串
string separator=";"; //终端读入多条命令的规定分隔符“；”


vector<string> history; //记录所有历史命令
vector<string> commands; // 所有命令,解析cmdline后得到的string数组
char *command[100];//切分后一条命令的字符串数组
int arg=0;//切分后一条命令的参数个数
char *argv[100][100];//解析单条命令command的参数
int ar=0;//管道个数
int tt;//记录当前遍历到第几个命令
int flag[100][3];//管道的输入输出以及追加重定向标记
char *file[100][2]={0};//对应两个重定向的文件
char * f=(char*)"temp.txt";//共享文件

bool back_flag; //是否后台执行

typedef void (CMD_HANDLER)(void);
typedef struct builtin_cmd
{
    char *name;
    CMD_HANDLER *handler;
} BUILTIN_CMD;

//全局函数声明
void init(); //初始化函数
void read_command(); //读取多种命令函数，主要处理“；”
void parse_command(); //处理一条命令command
void analazy_command(); //处理一条命令的多个参数，包括是否含有管道，输入输出定向
int execute_command(); //处理多管道，多线程，后台执行等
int do_command(int num); //执行命令，包括内建以及外部

//内部命令
int builtin(int num); //执行内部命令
void do_cd(); 
void do_history();
void do_echo();
void do_export();
void do_exit();

//内部命令解析,内部命令的结构体数组，用于处理内部命令
BUILTIN_CMD builtins[] = {
        {(char*)"cd", do_cd},
        {(char*)"history",do_history},
        {(char*)"echo",do_echo},
        {(char*)"export",do_export},
        {(char*)"exit",do_exit},
        {NULL, NULL}

};



int main()
{

    printf("\033[32m----------------welcome to mini shell--------2013747------ \n\033[0m");
    /* 获取用户id */
	uid = getuid();
    strcpy(current_dir, getcwd(NULL, 0));
    strcpy(user_dir, getcwd(NULL, 0));
//    printf("PATH : %s\n", getenv("PATH"));
    while (1)
    {
        printf("\033[92m%s@MINISHELL\033[0m:\033[34m%s\033[1;35m", getlogin(), current_dir);
         /* 打印用户提示符 */
        if (0 == uid)
        {
            printf("# \033[0m");		// 超级用户
        }
        else
        {
            printf("$ \033[0m");		// 普通用户
        }
        init();
        read_command();
        parse_command();

    }
    return 0;
}


void init(){
    //每次解析下一条命令前，要清空变量
    for (int i = 0; i < commands.size(); i++)
    {
        commands[i].clear();
    }
    cmdline.clear();
    commands.clear();
}

void read_command()
{
    //处理cmdline，根据separator把命令变成commands【】
    getline(cin, cmdline); // input string with ' '
    typedef string::size_type string_size;
    history.push_back(cmdline);
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
    //串行处理处理cmds
    for (int i = 0; i < commands.size(); i++)
    {
        for(int i=0;i<100;i++) command[i]=0;
		arg=0;//初始化参数个数
        string temp_command;
        stringstream input2(commands[i]); // string stream initialize 不按照空格划分
        //处理每一条commands【】，每一条命令可能有多参数，用child_commands【】来解析一条命令多参数
        while (input2 >> temp_command)
        {
            command[arg]=new char[temp_command.size()+1];
            strcpy(command[arg++],temp_command.c_str());
        }
        if(temp_command.compare("exit")==0){
            do_exit();
        }
        if (arg>0)
        {
            //这个函数就是处理分开参数之后的一条命令child_commands
            analazy_command();
            execute_command();
        }
    }
}


//解析命令
void analazy_command() {
	ar=0;
    tt=0;
	for(int i=0;i<100;i++) {
		flag[i][0]=flag[i][1]=flag[i][2]=0;
		file[i][0]=file[i][1]=0;
        back_flag=0;
		for(int j=0;j<100;j++) {
			argv[i][j]=0;
		}
	}
	for(int i=0;i<arg;i++) argv[0][i]=command[i];//初始化第一个参数
	argv[0][arg]=NULL;
	int a=0;//当前命令参数的序号
	for(int i=0;i<arg;i++) {
        //判断是否存在管道
		if(strcmp(command[i],"|")==0) {//c语言中字符串比较只能用strcmp函数
			//printf("遇到 | 符号\n");
			argv[ar][a++]=NULL;
			ar++;
			a=0;
		}
		else if(strcmp(command[i],"<")==0) {//存在输入重定向
			flag[ar][0]=1;
			file[ar][0]=command[i+1];
			argv[ar][a++]=NULL;
		}
		else if(strcmp(command[i],">")==0) {//没有管道时的输出重定向
			flag[ar][1]=1;
			file[ar][1]=command[i+1];
			argv[ar][a++]=NULL;//考虑有咩有输入重定向的情况
		}
        else if(strcmp(command[i],">>")==0){
            flag[ar][2]=1;
            file[ar][1]=command[i+1];
            argv[ar][a++]=NULL;
        }
        else if(strcmp(command[i],"&")==0){
            back_flag=1;
            argv[ar][a++]=NULL;
        }
        else argv[ar][a++]=command[i];
	}

}

int execute_command()
{
    int pid;
    pid=fork();//创建的子进程
	if(pid<0) {
		perror("fork error\n");
        exit(0);
	}
	//先判断是否存在管道，如果有管道，则需要用多个命令参数，并且创建新的子进程。否则一个命令即可
	else if(pid==0) {
        if(back_flag){
            //再判断是否存在后台运行符号，存在则重定向输入输出，对父进程发出信号，后台运行。
            freopen( "/dev/null", "w", stdout );
            freopen( "/dev/null", "r", stdin ); 
            signal(SIGCHLD,SIG_IGN);
        }
		if(!ar) {//没有管道
			if(flag[0][0]) {//判断有无输入重定向
				close(0);
				int fd=open(file[0][0],O_RDONLY);
			}
			if(flag[0][1]) {//判断有无输出重定向
				close(1);
				int fd2=open(file[0][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
			}
            if(flag[0][2]){
                close(1);
				int fd2=open(file[0][1],O_WRONLY|O_CREAT|O_APPEND,0666);
            }
			if(do_command(0)==-1){
                printf("command error! please retry!\n");
                exit(0);

            }
		}
		else {//有管道
			for(tt=0;tt<ar;tt++) {
				int pid2=fork();
				if(pid2<0) {
					perror("fork error\n");
					exit(0);
				}
				else if(pid2==0) {
					if(tt) {//如果不是第一个命令，则需要从共享文件读取数据
                        close(0);
					    int fd=open(f,O_RDONLY);//输入重定向
                    }
                    if(flag[tt][0]) {
						close(0);
						int fd=open(file[tt][0],O_RDONLY);
					}
					if(flag[tt][1]) {
						close(1);
						int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
					}
                    if(flag[tt][2])
                    {
                        close(1);
						int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_APPEND,0666);
                    }		
                    close(1);
                    remove(f);//由于当前f文件正在open中，会等到解引用后才删除文件
                    int fd=open(f,O_WRONLY|O_CREAT|O_TRUNC,0666);
					if(do_command(tt)==-1) {
                        printf("command error! please retry!\n");
                        exit(0);
                    }
				}
				else {//管道后的命令需要使用管道前命令的结果，因此需要等待
					waitpid(pid2,NULL,0);
				}
			}
            //接下来需要执行管道的最后一条命令
			close(0);
			int fd=open(f,O_RDONLY);//输入重定向
			if(flag[tt][1]) {
				close(1);
				int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
			}
            else if(flag[tt][2])
            {
                close(1);
                int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_APPEND,0666);
            }		
			if(do_command(tt)==-1) {
                printf("command error! please retry!\n");
                exit(0);
            }
		}
	}
	//father
	else {
        if(back_flag)
        {
            printf("[process id %d]\n",pid);
            return 1;
        }
        waitpid(pid,NULL,0);
	}
    return 1;

}

//内部命令解析
//执行返回1，没有表示0
int builtin(int num){
    int i = 0;
    int found = 0;
    while(builtins[i].name != NULL){
        if(strcmp(builtins[i].name,argv[num][0])==0) {
            builtins[i].handler();
            found=1;
            break;
        }
        i++;
    }
    return found;
}

int do_command(int num){
    if(builtin(num))
        return SUCCESS;
    int result=execvp(argv[num][0],argv[num]);
    return result;
}

void do_cd(){

        // 当前系统目录
        // char target_path[100];
        // getcwd(target_path, 100);
        // cout<<"now path"<<target_path<<endl;
        if (argv[tt][1] == 0)
        {
            strcpy(current_dir, user_dir);
            chdir(user_dir);
        }
        else
        {
            const char *rest = argv[tt][1];
            if (strcmp(argv[tt][1],"..")==0)
            {
                char *parent_dir = dirname(current_dir);
                strcpy(current_dir, parent_dir);
                chdir(parent_dir);
            }
            else if (strcmp(argv[tt][1],"~")==0)
            {
                strcpy(current_dir, user_dir);
                chdir(user_dir);
            }
            else 
            {
                if(chdir(rest)<0){
                    cout << "cd: " << rest << ":";
                    printf("\033[31m没有那个文件或目录.\n\033[0m");
                }
                else{
                    strcpy(current_dir, getcwd(NULL, 0));
                }
            }
            cout <<"current_dit"<< current_dir << endl;
        }
        exit(0);
    }



void do_history() {
    if (argv[tt][1]== 0)
    {
        for (int i = 0; i < history.size(); i++)
        {
            cout<<i+1<<' '<<history[i]<<endl;
        }
        
    }
    else if(strcmp(argv[tt][1],"-c")==0)
    {
        history.clear();
    }
    else if(int n = atoi(argv[tt][1]))
    {
        for(int i=history.size()-n; i<history.size(); i++)
        {
            cout<<i+1<<' '<<history[i]<<endl;
        }

    }
    exit(0);
}
void do_echo(){
    char* s = argv[tt][1];
    if(s == 0){
        fprintf(stderr, "echo: invalid argument\n");
        return;
    }
    //echo的格式：echo $PATH
    //1. echo $PATH 从环境变量中读取
    //2. echo PATH  直接输出echo后面的字符，如“PATH”
    if(s[0] == '$'){
        char* v = getenv(s + 1);
        printf("%s\n", v);
    }else{
        printf("%s\n", s);
    }
    exit(0);
}
void do_export(){
    //export格式：export CITY=ShangHai 
    if(argv[tt][1] == 0)
    { 
        fprintf(stderr, "export: invalid argument\n"); 
        return; 
    } 
    putenv(argv[tt][1]);
    exit(0);
}
void do_exit(){
    printf("\033[1;33m exit \033[0m\n");
    exit(0);
}