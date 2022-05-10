#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FORK_ERROR_CODE -1
#define FORKED_SUCCESS 0

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    char * command = argv[3];
    int child_pid = fork();

    if (child_pid == FORK_ERROR_CODE)
        return 1;

    if (child_pid == FORKED_SUCCESS) {
        char temporaryDirPath[] = "./temp";
        int temporaryDirPermissions = 700;

        mkdir(temporaryDirPath, temporaryDirPermissions);
        chroot(temporaryDirPath);

        execv(command, &argv[3]);
    } else {
        int child_status;
        wait(&child_status);
        return WEXITSTATUS(child_status);
    }

    return 0;
}
