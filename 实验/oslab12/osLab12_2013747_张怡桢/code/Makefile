ModuleName=lkm_example
obj-m +=${ModuleName}.o
all:${ModuleName}.ko
${ModuleName}.ko:${ModuleName}.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
test:${ModuleName}.ko
	echo make test_ins
	echo make test_mk
	echo make test_test
	echo make test_rm
test_ins:${ModuleName}.ko
	sudo dmesg -C
	sudo insmod ${ModuleName}.ko name=/home/parallels/Documents/lab12/zyz
	sudo dmesg
test_mk:${ModuleName}.ko
	sudo mknod /dev/${ModuleName} c 509 0
test_read:${ModuleName}.ko
	sudo dmesg -C
	cat /dev/${ModuleName}
	sudo dmesg
test_write:${ModuleName}.ko
	echo "hello 2013747 zhangyizhen 2022/12/8" >> /dev/${ModuleName}
test_rm:${ModuleName}.ko
	sudo rmmod ${ModuleName}.ko
	sudo dmesg
.PHONY:clean
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rm /dev/lkm_example
