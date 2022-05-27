#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/utsname.h>
#include <sys/mman.h>

#define FORK_ERROR_CODE -1
#define FORKED_SUCCESS 0
#define DEFAULT_BUFFER_SIZE 1024
#define CHILD_STACK_SIZE (1024 * 1024)

#define TEMP_DIR_PATH "temp/"
#define TEMP_DIR_PERMISSIONS 0777
#define TEMP_EXEC_NAME "tmp_exec"

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

    FILE * targetFile = fopen(toPath, "wb");

    if (targetFile == NULL) {
        printf("Error! Cannot create target file\n");
        return -1;
    }

    int byte;
    unsigned char buffer[DEFAULT_BUFFER_SIZE];
    do {
        byte = fread(buffer, 1, sizeof buffer, sourceFile);
        if (byte) {
            byte = fwrite(buffer, 1, byte, targetFile);
        } else {
            byte = 0;
        }
    } while (byte > 0);

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

int handle_child_process(void * args) {

    char ** argv = (char**)(args);

    if (!dir_exist(TEMP_DIR_PATH)) {
        mkdir(TEMP_DIR_PATH, TEMP_DIR_PERMISSIONS);
    }

    char * command = argv[3];
    char targetPathPrefix[DEFAULT_BUFFER_SIZE];
    strcpy(targetPathPrefix, TEMP_DIR_PATH);
    char * targetPath = strcat(targetPathPrefix, basename(TEMP_EXEC_NAME));


    int copyingStatus = copyFile(command, targetPath);
    if (copyingStatus == -1) {
        printf("Cannot copying files\n");
        exit(-1);
    }

    chdir(TEMP_DIR_PATH);
    if (chroot(".") != 0) {
        printf("Failed chroot %s\n", TEMP_DIR_PATH);
        exit(-1);
    }

    char * tempCommandName = basename(TEMP_EXEC_NAME);
    int changeModStatus = chmod(tempCommandName, TEMP_DIR_PERMISSIONS);

    if (changeModStatus != 0) {
        perror("Error when try change file permissions");
        exit(changeModStatus);
    }

    char * temporaryArguments[] = {
            TEMP_EXEC_NAME,
            argv[4],
            argv[5],
            NULL
    };

    int executionStatus = execv(tempCommandName, temporaryArguments);
    perror("Error when try execute command");
    exit(executionStatus);
}


int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    char * stack;
    char * stackTop;
    int childPid;
    struct utsname uts;

    stack = mmap(NULL, CHILD_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }
    stackTop = stack + CHILD_STACK_SIZE;
    childPid = clone(handle_child_process, stackTop, CLONE_NEWPID | SIGCHLD, argv);


    if (childPid == FORK_ERROR_CODE)
        return 1;

    if (childPid == FORKED_SUCCESS) {
        handle_child_process(argv);
    }

    int childStatus;
    wait(&childStatus);
    cleanup_environment();
    return WEXITSTATUS(childStatus);
}
