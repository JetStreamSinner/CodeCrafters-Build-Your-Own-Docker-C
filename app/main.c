#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define FORKING_ERROR -1
#define FORKING_SUCCESS 0

// Usage: your_docker.sh run <image> <command> <arg1> <arg2> ...
int main(int argc, char *argv[]) {
    printf("XER");
    setbuf(stdout, NULL);

    char * command = argv[3];

    int child_pid = fork();

    if (child_pid == FORKING_ERROR) {
        return 1;
    }

    if (child_pid == FORKING_SUCCESS) {
        execv(command, &argv[3]);
    } else {
        wait(NULL);
    }

    return 0;
}
