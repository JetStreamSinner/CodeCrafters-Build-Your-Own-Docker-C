#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define FORKING_ERROR -1
#define INHERITOR_PID 0

// Usage: your_docker.sh run <image> <command> <arg1> <arg2> ...
int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    char * command = argv[3];
    int child_pid = fork();

    switch (child_pid) {
        case FORKING_ERROR:
            return 1;
        case INHERITOR_PID:
            execv(command, &argv[3]);
            break;
        default: {
            int child_status;
            wait(&child_status);
            return WEXITSTATUS(child_status);
        }
    }

    return 0;
}
