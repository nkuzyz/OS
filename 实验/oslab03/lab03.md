# Download and compile the latest linux kernel

## Target
1. Download the latest linux kernel

2. config the latest linux kernel

3. compile the latest linux kernel

4. boot ubuntu with the latest linux kernel

## preparations

### install related software package

```
sudo apt-get update
sudo apt-get upgrade
```

```
sudo apt-get install build-essential
```

```
sudo apt-get install wget
```

```
sudo apt-get install pkg-config
sudo apt-get install libgtk2.0-dev
sudo apt-get install libcanberra-gtk-module
sudo apt-get install glade libglade2-dev
```

```
sudo apt-get install flex bison
sudo apt-get install libssl-dev
sudo apt-get install libelf-dev
sudo apt-get install libncurses-dev
```

### view the current linux kernel version
```
uname -a
```

![image-20221007191514517](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007191514517.png)

## How to do

Download and compile the latest linux kernel
### the latest linux kernel
Linux Stable Kernel: 5.14.14  (2021.10.26)

### 1. Download the latest linux kernel from www.kernel.org

* [linux kernel 5.14.14](https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.14.14.tar.xz)

```
cd ~
pwd
```

```
wget -c https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.14.14.tar.xz
```

```
tar xvJf linux-5.14.14.tar.xz
```

```
sudo ln -s `pwd`/linux-5.14.14 /usr/src/linux
```

![image-20221007192118255](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007192118255.png)

### 2. config the latest linux kernel

```
cd /usr/src/linux
cp /boot/config-`uname -r`   .config
```

```
make localmodconfig 或 make oldconfig
make gconfig
```

### 3. customized config

#### 3.1. add local version

add your ID as local version

![image-20221007200243750](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007200243750.png)

#### 3.2. some errors

##### 3.2.2 No rule to make target ‘debian/canonical-certs.pem‘, needed by ‘certs/x509_certificate_list

solution: set CONFIG_SYSTEM_TRUSTED_KEYS=""

**搜索config_system_trusted_keys（这个我的引号里已经是空的了）和config_system_revocation_keys（应该不区分大小写），将它们“=”后的双引号里的内容删掉，右上角保存**

in crytoXXX item of config

终于搞定了，我的电脑在make localmodconfig后一路回车，没执行make gconfig（老师的意思好像是这步就是为了修改内核名称，但好像就是为了截图里姓名学号全拼，我的用户名就是这个就没改），之后输入gedit .config，在打开的文件中按左ctrl+f在右上角呼出搜索框，搜索config_system_trusted_keys（这个我的引号里已经是空的了）和config_system_revocation_keys（应该不区分大小写），将它们“=”后的双引号里的内容删掉，右上角保存，另外config_module_sig_key里的内容没删，其他操作完全按照老师昨天八点半发的pdf里的步骤弄就可以了好像。


### (optional)4. clean the previous compilings

If you want to recompile , please execute the following command to clean the previous work:

```
make clean
```

### 5. compile the latest linux kernel (long wait)

compile the latest linux kernel
```
make -j8
```
please compile again: (why?也许还有个坑...，必须编译完全正确，很重要)

```
sudo make
```

继续编译模块:(当前内核已经没有这个步骤了)

```
sudo make modules
```

install the latest linux kernel

```
sudo make modules_install
sudo make install
```

Successfully compiled:



### 6. boot ubuntu with the latest linux kernel

```
sudo reboot
```

```
uname -a
```

![image-20221007200045127](https://zyzstc-1303973796.cos.ap-beijing.myqcloud.com/uPic/image-20221007200045127.png)

End.