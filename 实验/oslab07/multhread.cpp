#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stack>
#include <dirent.h>
#include <cstring>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
const int PATHLEN=1024;
const int THREAD_NUM=20;
const int BUF_SIZE=1024;
pthread_mutex_t amutex;

class Container
{
public:
    string srcFile;
    string dstFile;
    bool ifLink;
};
stack<Container> containerStack; //存放待复制的普通文件的源地址和目的地址的栈
//遍历目录
void walk_dir(char *srcDir, char *dstDir)
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
                //若该文件夹是一个软连接，将其ifLink设为true，入栈
                if (ptr->d_type == 10)
                {
                    struct Container new_container;
                    char *linkFile = new char[PATHLEN];
                    readlink(subSrcPath, linkFile, 1024);
                    new_container.srcFile = linkFile;
                    new_container.dstFile = subDstPath;
                    new_container.ifLink = true;
                    containerStack.push(new_container);
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
                //若该文件是一个软连接，将其ifLink设为true，入栈
                if (ptr->d_type == 10)
                {
                    char *linkFile = new char[PATHLEN];
                    readlink(srcFile, linkFile, 1024);
                    new_container.srcFile = linkFile;
                    new_container.ifLink = true;
                    containerStack.push(new_container);
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

//复制文件
void copyfile(string srcFile, string dstFile)
{
    int infd, outfd;
    char buffer[BUF_SIZE];
    int i;
    if ((infd = open(srcFile.c_str(), O_RDONLY)) < 0)
    {
        cout << "Open srcFile failed:" << srcFile.c_str() << endl;
        return;
    }
    if ((outfd = open(dstFile.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0)
    {
        cout << "Open dstFile failed:" << dstFile.c_str() << endl;
        return;
    }
    // cout << "Copyfile from " << srcFile << " to " << dstFile << endl;
    while (1)
    {
        i = read(infd, buffer, BUF_SIZE);
        if (i <= 0)
            break;
        write(outfd, buffer, i);
    }
    close(outfd);
    close(infd);
}

//每个线程运行的函数
void *run(void *)
{
    Container tmp;
    while (containerStack.empty() == false)
    {
        //上锁，各线程动态获取拷贝任务
        pthread_mutex_lock(&amutex);
        if (containerStack.empty() == true)
            break;
        tmp = containerStack.top();
        containerStack.pop();
        pthread_mutex_unlock(&amutex);
        //若是软连接，则调用symlink接口创建软连接
        if (tmp.ifLink == true)
        {
            // cout << "Create link:" << tmp.srcFile.c_str() << " link to " << tmp.dstFile.c_str() << endl;
            symlink(tmp.srcFile.c_str(), tmp.dstFile.c_str());
        }
        //若是普通文件，则调用copyfile函数拷贝文件
        else
        {
            copyfile(tmp.srcFile, tmp.dstFile);
        }
    }
    return NULL;
}


int main(int argc, char **argv)
{
    walk_dir(argv[1], argv[2]);
    //初始化互斥量
    pthread_mutex_init(&amutex, NULL);
    //创建线程
    pthread_t threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&threads[i], NULL, run, NULL);
    }
    //回收各线程
    for (int i = 0; i < THREAD_NUM; i++)
    {
        cout << i << endl;
        pthread_join(threads[i], NULL);
    }
    //销毁互斥量
    pthread_mutex_destroy(&amutex);
    return 0;
}
