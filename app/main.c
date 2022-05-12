#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#define FORK_ERROR_CODE -1
#define FORKED_SUCCESS 0
#define DEFAULT_BUFFER_SIZE 1024

#define TEMP_DIR_PATH "temp/"
#define TEMP_DIR_PERMISSIONS 0777

int dir_exist(char * path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        return 0;
    }
    return 1;
}

void cleanup_environment() {
    system("rm -rf temp");
}

int copyFile(char * fromPath, char * toPath) {
    FILE * sourceFile = fopen(fromPath, "rb");

    if (sourceFile == NULL) {
        printf("Error! Cannot open source file\n");
        return -1;
    }

    FILE * targetFile = fopen(toPath, "wb+");

    if (targetFile == NULL) {
        printf("Error! Cannot create target file\n");
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

void print_work_directory() {
    char currentWorkDirectory[DEFAULT_BUFFER_SIZE];
    getcwd(currentWorkDirectory, DEFAULT_BUFFER_SIZE);
    printf("Current work directory: %s\n", currentWorkDirectory);
}

void handle_child_process(char * command, char * argv[]) {
    if (!dir_exist(TEMP_DIR_PATH)) {
        mkdir(TEMP_DIR_PATH, TEMP_DIR_PERMISSIONS);
    }

    char targetPathPrefix[DEFAULT_BUFFER_SIZE];
    strcpy(targetPathPrefix, TEMP_DIR_PATH);
    char * targetPath = strcat(targetPathPrefix, basename(command));


    int copyingStatus = copyFile(argv[3], targetPath);
    if (copyingStatus == -1) {
        printf("Cannot copying files\n");
        exit(-1);
    }

    if (chroot(TEMP_DIR_PATH) != 0) {
        printf("Failed chroot %s\n", TEMP_DIR_PATH);
        exit(-1);
    }
    chdir(TEMP_DIR_PATH);
    execv(command, &argv[3]);
}


int main(int argc, char *argv[]) {

    setbuf(stdout, NULL);
    char * command = argv[3];
    int childPid = fork();

    if (childPid == FORK_ERROR_CODE)
        return 1;

    if (childPid == FORKED_SUCCESS) {
        handle_child_process(command, argv);
    }

    int childStatus;
    wait(&childStatus);
//    cleanup_environment();
    return WEXITSTATUS(childStatus);
}
