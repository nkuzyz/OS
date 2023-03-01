# Add a New System Call

## Target
1. Add a new system call into the linux kernel

2. Test the new system call in user model


## Tools

### Install GCC Software Colletion
```
sudo apt-get install build-essential
```
### How to use GCC
* [gcc and make](https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html)


## How to do
see the pdf document: newsyscall2020.pdf

### Step0 

new customized kernel config: [config1](config_ubuntu2004_20211129), [config2](config_5_14_14_ubuntuok)

### Step1 (Linux kernel 5.19)

include/linux/syscalls.h

在文件(No. 1279)
#endif /* CONFIG_ARCH_HAS_SYSCALL_WRAPPER */之前，添加一行:

asmlinkage long sys_schello(void);

![image-20221007185905209](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007185905209.png)

![image-20221007200704635](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007200704635.png)



### Step2 (Linux kernel 5.19)

kernel/sys.c
在文件SYSCALL_DEFINE0(gettid)函数之后（No. 949），添加如下行:

SYSCALL_DEFINE0(schello)
{
printk("Hello new system call schello!Your ID\n");
return 0;
}

![image-20221007201013494](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007201013494.png)

### Step3  (Linux kernel 5.19)

针对64位OS
arch/x86/entry/syscalls/syscall_64.tbl
在文件334 common  memfd_secret        sys_memfd_secret 行之后，添加如下行:

​         335 common schello                   sys_schello

![image-20221007211303093](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007211303093.png)

由于我的电脑不是X86架构的，所以我按着这个思路修改了arm/tools下的syscall.tbl，但是编译之后并不能生效，在最后的输出依然失败，询问同学，查询网站，找到解决办法【https://blog.csdn.net/m0_51683653/article/details/124133370】

![image-20221007215028987](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007215028987.png)

在889行加入

```c
#define __NR_schello 451
__SYSCALL(__NR_schello,sys_schello)
```

并顺次把下一个syscalls序号加1改成452即可。

![image-20221007222303687](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007222303687.png)

### Step4

make oldconfig

make gconfig

在这一步我在版本名上把原来的2013747改成了zyz2013747以区别更改之后的新版本

make clean
make -j5

// make -jx（x越大，编译的时候用的线程越多，就越快！！！！！）

![image-20221007223505321](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007223505321.png)

sudo make

sudo make modules

sudo make modules_install
sudo make install

### Step 5

重新启动:

sudo reboot

可以看到有zyz2013747后缀的新版本，选择进入即可

![image-20221007230635543](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007230635543.png)

确认新内核是否成功运行：

```
uname -a
```

![image-20221007224802950](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007224802950.png)


### Step 6

编写用户态测试程序testschello.c

```
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#define __NR_schello 451
int main(int argc, char *argv[])
{
 syscall(__NR_schello);
 printf("ok! run dmesg | grep hello in terminal!\n");
 return 0;
}
```



### Step 7

编译用户态测试程序testschello.c，并执行

```
gcc -o testsc testschello.c

$ sudo dmesg -C

./testsc

$ sudo dmesg | grep schello

[ 152.075153] Hello new system call schello! 2013747 zhangyizhen

```

![image-20221007225021753](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007225021753.png)

### End.