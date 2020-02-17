#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

// File size in bytes
#define FILE_SIZE 65536
#define MAX_MESSAGE_SIZE 100

void strToHEXStr(char* input, char* output)
{
    int counter;
    counter=0;
    while(input[counter] != '\0')
    {
        sprintf((char*)(output+2*counter),"%02X", input[counter]);
        counter++;
    }
    //insert NULL at the end of the output string
    output[2*counter] = '\0';
}

void writeToSharedMemory(void* sharedMemoryPointer, char* message) {
    sprintf(sharedMemoryPointer, "%s", message);
}

int main(void) {

    const char doneMessage[] = "done";

    int parentToChildAPipe[2];
    if (pipe(parentToChildAPipe)) {
        perror("Parent -> ChildA Pipe failed");
    }

    int childAToParentPipe[2];
    if (pipe(childAToParentPipe)) {
        perror("Child A -> Parent Pipe failed");
    }

    pid_t childAPID = fork();
    int sharedMemoryAFD;
    void *sharedMemAPtr;
    sharedMemoryAFD = shm_open("sharedMemoryA", O_CREAT | O_RDWR, 0666);
    ftruncate(sharedMemoryAFD, FILE_SIZE);
    sharedMemAPtr = mmap(NULL, FILE_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryAFD, 0);

    if (childAPID == 0) {
        // child a will run this part
        close(parentToChildAPipe[1]); // Close write end of parent -> childA pipe
        close(childAToParentPipe[0]); // Close read end of childA -> parent pipe
        char childAMessageFromParent[FILE_SIZE];
        read(parentToChildAPipe[0], childAMessageFromParent, FILE_SIZE);
        close(parentToChildAPipe[0]); // Close read end of parent -> child pipe
        writeToSharedMemory(sharedMemAPtr, childAMessageFromParent);
        write(childAToParentPipe[1], doneMessage, sizeof(doneMessage));
        close(childAToParentPipe[1]); // Close read end of parent -> child pipe
    }
    else if (childAPID < 0) {
        /* The fork failed. */
        perror("Fork failed");
    }
    else {
        // parent will run this part
        int parentToChildBPipe[2];
        if (pipe (parentToChildBPipe)) {
            perror("Parent -> ChildB Pipe failed");
        }

        int childBToParentPipe[2];
        if (pipe(childBToParentPipe)) {
            perror("ChildB -> Parent Pipe failed");
        }

        pid_t childBPID = fork();

        int sharedMemoryBFD;
        void *sharedMemBPtr;
        sharedMemoryBFD = shm_open("sharedMemoryB", O_CREAT | O_RDWR, 0666);
        ftruncate(sharedMemoryBFD, FILE_SIZE * 2);
        sharedMemBPtr =  mmap(NULL, FILE_SIZE * 2, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryBFD, 0);

        if (childBPID == 0) {
            // child b will run this part
            close(parentToChildBPipe[1]); // Close write end of parent -> child pipe
            close(childBToParentPipe[0]); // Close read end of childB -> parent pipe
            // close child a related pipes
            close(childAToParentPipe[0]);
            close(childAToParentPipe[1]);
            close(parentToChildAPipe[0]);
            close(parentToChildAPipe[1]);

            char childBMessageFromParent[FILE_SIZE];
            read(parentToChildBPipe[0], childBMessageFromParent, FILE_SIZE);
            close(parentToChildBPipe[0]); // Close read end of parent -> child pipe

            char messageConvertedToHEX[2*FILE_SIZE];
            strToHEXStr(childBMessageFromParent, messageConvertedToHEX);
            writeToSharedMemory(sharedMemBPtr, messageConvertedToHEX);
            write(childBToParentPipe[1], doneMessage, sizeof(doneMessage));
            close(childBMessageFromParent[1]);
        }
        else if (childBPID < 0) {
            /* The fork failed. */
            perror("Fork failed");
        }
        else {
            // parent will run this part
            close(parentToChildAPipe[0]); // Close read end of parent -> child a pipe
            close(parentToChildBPipe[0]); // Close read end of parent -> child b pipe
            close(childAToParentPipe[1]); // Close write end of childA -> parent pipe
            close(childBToParentPipe[1]); // Close write end of childB -> parent pipe

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
            close(parentToChildAPipe[1]); // Close write end of parent -> child a pipe
            close(parentToChildBPipe[1]); // Close write end of parent -> child b pipe

            char messageFromChild[MAX_MESSAGE_SIZE];
            read(childAToParentPipe[0], messageFromChild, MAX_MESSAGE_SIZE);
            close(childAToParentPipe[0]);
            if (strcmp(messageFromChild, doneMessage) == 0) {
                printf("Output from child A: %s\n", (char*) sharedMemAPtr);
            }
            else {
                perror("ChildA sent a message other than 'done'.");
            }

            read(childBToParentPipe[0], messageFromChild, MAX_MESSAGE_SIZE);
            close(childBToParentPipe[0]);
            if (strcmp(messageFromChild, doneMessage) == 0) {
                fprintf(stderr, "Output from child B: %s\n", (char*) sharedMemBPtr);
            }
            // release the shared memories
            shm_unlink("sharedMemoryA");
            shm_unlink("sharedMemoryB");
            wait(NULL);
        }
    }
    return  0;
}