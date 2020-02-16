#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

// File size in bytes
#define FILE_SIZE 65536

int main(void) {
    pid_t childAPID = fork();

    int sharedMemoryAFD, sharedMemoryBFD;
    void *sharedMemAPtr, *sharedMemBPtr;

    sharedMemoryAFD = shm_open("sharedMemoryA", O_CREAT | O_RDWR, 0666);
    sharedMemoryBFD = shm_open("sharedMemoryB", O_CREAT | O_RDWR, 0666);
    ftruncate(sharedMemoryAFD, FILE_SIZE);
    ftruncate(sharedMemoryBFD, FILE_SIZE * 2);
    sharedMemAPtr = mmap(NULL, FILE_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryAFD, 0);
    sharedMemBPtr =  mmap(NULL, FILE_SIZE * 2, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryBFD, 0);

    int parentToChildAPipe[2];
    if (pipe (parentToChildAPipe))
    {
        perror("Parent -> ChildA Pipe failed");
    }

    if (childAPID == 0) {
        // child a will run this part
        close(parentToChildAPipe[1]); // Close write end of parent -> child pipe
        char messageFromParent[FILE_SIZE];
        read(parentToChildAPipe[0], messageFromParent, FILE_SIZE);
        printf("Hello from child A\n");
        close(parentToChildAPipe[0]); // Close read end of parent -> child pipe
        printf("Child A: %s\n", messageFromParent);
    }
    else if (childAPID < 0) {
        /* The fork failed. */
        perror("Fork failed");
    }
    else {
        // parent will run this part
        int parentToChildBPipe[2];
        if (pipe (parentToChildBPipe))
        {
            perror("Parent -> ChildB Pipe failed");
        }
        pid_t childBPID = fork();
        if (childBPID == 0) {
            // child b will run this part
            close(parentToChildBPipe[1]); // Close write end of parent -> child pipe

            char messageFromParent[FILE_SIZE];
            read(parentToChildBPipe[0], messageFromParent, FILE_SIZE);
            printf("Hello from child B\n");
            close(parentToChildBPipe[0]); // Close read end of parent -> child pipe
            printf("Child B: %s\n", messageFromParent);
        }
        else if (childBPID < 0) {
            /* The fork failed. */
            perror("Fork failed");
        }
        else {
            // parent will run this part
            close(parentToChildAPipe[0]); // Close read end of parent -> child a pipe
            close(parentToChildBPipe[0]); // Close read end of parent -> child b pipe

            FILE *filePtr;
            char fileContent[FILE_SIZE];
            char contentBuffer[FILE_SIZE];

            filePtr = fopen("input.txt", "r");
            if (filePtr == NULL) {
                perror("Could not open file input.txt");
                return -1;
            }

            while (fgets(contentBuffer, FILE_SIZE, filePtr)) {
                strcat(fileContent, contentBuffer);
            }
            // send file contents to children
            write(parentToChildAPipe[1], fileContent, FILE_SIZE);
            write(parentToChildBPipe[1], fileContent, FILE_SIZE);
            printf("Str len: %lu\n", strlen(fileContent));
            close(parentToChildAPipe[1]); // Close write end of parent -> child a pipe
            close(parentToChildBPipe[1]); // Close write end of parent -> child b pipe
            wait(NULL);
            wait(NULL);

        }
    }
    return  0;
}