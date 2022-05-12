#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#define FORK_ERROR_CODE -1
#define FORKED_SUCCESS 0


int copyFile(char * fromPath, char * toPath) {
    FILE * sourceFile = fopen(fromPath, "rb");

    if (sourceFile == NULL) {
        printf("Error! Cannot open source file");
        return -1;
    }

    FILE * targetFile = fopen(toPath, "wb+");

    if (targetFile == NULL) {
        printf("Error! Cannot create target file");
        return -1;
    }

    char byte = fgetc(sourceFile);
    while (byte != EOF) {
        fputc(byte, targetFile);
        byte = fgetc(sourceFile);
    }

    fclose(sourceFile);
    fclose(targetFile);

    return 0;
}

void print_args(int argumentsCount, char * arguments[]) {
    for (int argumentIndex = 0; argumentIndex < argumentsCount; ++argumentIndex)
        printf("[%d] -> %s\n", argumentIndex, arguments[argumentIndex]);
}

int main(int argc, char *argv[]) {

    setbuf(stdout, NULL);
    char * command = argv[3];
    int childPid = fork();

    if (childPid == FORK_ERROR_CODE)
        return 1;

    if (childPid == FORKED_SUCCESS) {
        char temporaryDirPath[] = "temp/";
        int temporaryDirPermissions = 0777;

        struct stat st;
        if (stat(temporaryDirPath, &st) == -1) {
            mkdir(temporaryDirPath, temporaryDirPermissions);
        }

        char targetPathPrefix[] = "temp/";
        char * targetPath = strcat(targetPathPrefix, basename(argv[3]));
        int copyingStatus = copyFile(argv[3], targetPath);

        if (copyingStatus == -1) {
            exit(-1);
        }

//        char isolationDir[] = "/temp";
//        if (chroot(isolationDir) != 0) {
//            printf("Failed chroot %s\n", isolationDir);
//            exit(-1);
//        }
        execv(command, &argv[3]);
    }

    int childStatus;
    wait(&childStatus);
    return WEXITSTATUS(childStatus);
}
