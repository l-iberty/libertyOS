#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
    if (pid > 0) {
        printf("parent [%d,%d]\n", getpid(), getppid());
    }
    else if (pid == 0) {
        printf("child [%d,%d]\n", getpid(), getppid());
    }
}
