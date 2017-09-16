#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd1, fd2;

    //fd2 = open("file1", O_RDWR);
    //printf("%d\n",fd2);
    //close(fd2);
    unlink("file1");
}
