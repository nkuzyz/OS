#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
using namespace std;
const int BUF_SIZE = 1024;

int main(int argc, char *argv[])
{
	int infd, outfd;
	char buffer[BUF_SIZE];
	int i;
	if ((infd = open(argv[1], O_RDONLY)) < 0)
	{
		exit(0);
	}
	if ((outfd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0)
	{
		exit(1);
	}
	// cout << "Copyfile from " << argv[1] << " to " << argv[2] << endl;
	while (1)
	{
		i = read(infd, buffer, BUF_SIZE);
		if (i <= 0)
			break;
		write(outfd, buffer, i);
	}
	close(outfd);
	close(infd);
	exit(0);
}
