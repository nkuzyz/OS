#include<iostream>
#include<stack>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include <csignal>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

const int PATHLEN=1024;
const int PROCESS_NUM=1;

struct Container
{

	char *srcFile;
	char *dstFile;
};
stack<Container> containerStack; //存放待复制的普通文件的源地址和目的地址的栈
stack<Container> linkStack;		 //存放软连接连接内容和连接地址的栈

//遍历目录
void walk_dir(const char *srcDir, const char *dstDir)
{
	DIR *dir;
	struct dirent *ptr;
	//打开目录，并判断打开是否成功
	dir = opendir(srcDir);
	if (dir == NULL)
	{
		cout << "打开 srcDir 失败" << endl;
		exit(0);
	}
	//提取文件夹信息
	char *path = new char[PATHLEN];
	while ((ptr = readdir(dir)) != NULL)
	{
		//遇到. ..就跳过
		if ((!strcmp(ptr->d_name, ".")) || (!strcmp(ptr->d_name, "..")))
		{
			continue;
		}
		struct stat s;
		snprintf(path, PATHLEN, "%s/%s", srcDir, ptr->d_name);
		lstat(path, &s);
		if (stat(path, &s) == 0)
		{
			//若该地址是一个文件夹
			if (S_ISDIR(s.st_mode))
			{
				char *subSrcPath = new char[PATHLEN];
				char *subDstPath = new char[PATHLEN];
				snprintf(subSrcPath, PATHLEN, "%s/%s", srcDir, ptr->d_name);
				snprintf(subDstPath, PATHLEN, "%s/%s", dstDir, ptr->d_name);
				//若该文件夹是一个软连接
				if (ptr->d_type == 10)
				{
					struct Container new_container;
					char *linkFile = new char[PATHLEN];
					readlink(subSrcPath, linkFile, 1024);
					new_container.srcFile = linkFile;
					new_container.dstFile = subDstPath;
					linkStack.push(new_container);
					continue;
				}
				//在目标地址处创建文件夹
				mkdir(subDstPath, S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
				//递归遍历子目录
				walk_dir(subSrcPath, subDstPath);
			}
			//若该地址是一个普通文件
			else if (S_ISREG(s.st_mode))
			{
				char *srcFile = new char[PATHLEN];
				char *dstFile = new char[PATHLEN];
				snprintf(srcFile, PATHLEN, "%s/%s", srcDir, ptr->d_name);
				snprintf(dstFile, PATHLEN, "%s/%s", dstDir, ptr->d_name);
				//将待拷贝的文件的源地址和目的地址打包放入栈中
				struct Container new_container;
				new_container.srcFile = srcFile;
				new_container.dstFile = dstFile;
				//若该文件是一个软连接
				if (ptr->d_type == 10)
				{
					char *linkFile = (char *)malloc(sizeof(char) * PATHLEN);
					readlink(srcFile, linkFile, 1024);
					new_container.srcFile = linkFile;
					linkStack.push(new_container);
					continue;
				}
				containerStack.push(new_container);
			}
			else
			{
				cout << "not file not directory" << endl;
				exit(0);
			}
		}
		else
		{
			cout << "error, path doesn't exist" << endl;
			exit(0);
		}
	}
	closedir(dir);
}


int main(int argc, char **argv)
{

	walk_dir(argv[1], argv[2]);
	while (linkStack.empty()==false)
	{
		Container tmp=linkStack.top();
		linkStack.pop();
		cout<<"Create link: "<<tmp.srcFile<<"<-----"<<tmp.dstFile<<endl;
		symlink(tmp.srcFile,tmp.dstFile);
	}
	

	//创建单进程复制文件
	pid_t pid[PROCESS_NUM];
	int cnt = 0;
	while (containerStack.empty() == false)
	{
		Container tmp = containerStack.top();
		containerStack.pop();
		//限制创建的进程总数不超过PROCESS_NUM
		while (cnt == PROCESS_NUM)
		{
			wait(NULL);
			cnt--;
		}
		pid[cnt] = fork();
		if (pid[cnt] < 0)
		{
			fprintf(stderr, "InParent: Fork Failed\n");
			printf("errno=%d\n", errno);
			exit(-1);
		}
		else if (pid[cnt] == 0)
		{
			//子进程调用copyfile程序，完成普通文件复制
			execlp("//home/parallels/Documents/Program/lab06_mul/copyfile", "./copyfile", tmp.srcFile, tmp.dstFile, NULL);
		}
		cnt++;
	}

	//等待所有子进程完成
	while (cnt != 0)
	{
		printf("In parent: Waiting the child %d\n", cnt);
		wait(NULL);
		cnt--;
	}
}