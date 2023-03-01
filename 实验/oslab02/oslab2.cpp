#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include <csignal>

void copy_file(char *sour, char *targ) {
    //复制文件
    char *buffer = (char *) malloc(1024);
    FILE *in, *out;
    if ((in = fopen(sour, "r")) == NULL) {
        exit(1);
    }
    if ((out = fopen(targ, "w")) == NULL) {
        exit(1);
    }
    int len;
    while ((len = fread(buffer, 1, 1024, in)) > 0) {
        fwrite(buffer, 1, len, out);
    }
    fclose(out);
    fclose(in);
    free(buffer);
}

void get_file(const char *path, const char *target) {
    if (opendir(target) == nullptr) {
    //如果要拷贝的文件夹路径不存在，则新建文件夹并赋予权限777
        mkdir(target, 0777);
    }
    DIR *dir;
    struct dirent *ptr;
    //打开原文件夹
    dir = opendir(path);
    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr->d_type == DT_DIR) {
            //目录
            if (strcmp(ptr->d_name, "..") != 0 && strcmp(ptr->d_name, ".") != 0) {
                char *src = (char *) malloc(512);
                src = strcpy(src, path);
                src = strcat(src, "/");
                src = strcat(src, ptr->d_name);
                char *targ = (char *) malloc(512);
                targ = strcpy(targ, target);
                targ = strcat(targ, "/");
                targ = strcat(targ, ptr->d_name);
                if (opendir(targ) == nullptr) {//
                    mkdir(targ, 0777);
                }
                get_file(src, targ);
                free(src);
                free(targ);
            }
        } else if (ptr->d_type == DT_REG) {
            //常规文件
            {
                char *filename = (char *) malloc(512);
                filename = strcpy(filename, path);
                filename = strcat(filename, "/");
                filename = strcat(filename, ptr->d_name);
                char *target_name = (char *) malloc(512);
                target_name = strcpy(target_name, target);
                target_name = strcat(target_name, "/");
                target_name = strcat(target_name, ptr->d_name);
                copy_file(filename, target_name);
                free(filename);
                free(target_name);
            }
        } else {
            if (strcmp(ptr->d_name, "..") != 0 && strcmp(ptr->d_name, ".") != 0) {
//                printf("%s\n", ptr->d_name);
                char *src = (char *) malloc(512);
                src = strcpy(src, path);
                src = strcat(src, "/");
                src = strcat(src, ptr->d_name);
                char *targ = (char *) malloc(512);
                targ = strcpy(targ, target);
                targ = strcat(targ, "/");
                targ = strcat(targ, ptr->d_name);
                if (opendir(targ) == nullptr) {//
                    link(src, targ);
                }
                free(src);
                free(targ);
            }
        }
    }
}


int main(int argc, char **argv) {
    get_file(argv[1], argv[2]);

    exit(0);
}