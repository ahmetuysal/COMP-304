#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

// File size in bytes
#define CHUNK_SIZE 65536
#define MAX_MESSAGE_SIZE 100
#define MAX_FILE_PATH_LENGTH 4096

void strToHEXStr(char* input, char* output)
{
    int counter;
    counter=0;
    while(input[counter] != '\0')
    {
        sprintf((char*)(output+2*counter),"%02X", input[counter]);
        counter++;
    }
    // insert NULL at the end of the output string
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

    if (childAPID == 0) {
        // child a will run this part
        close(parentToChildAPipe[1]); // Close write end of parent -> childA pipe
        close(childAToParentPipe[0]); // Close read end of childA -> parent pipe

        long fileSize;
        read(parentToChildAPipe[0], &fileSize, sizeof(fileSize));

        char *fileContent = malloc( sizeof(char) * ( fileSize + 1 ) );

        long counter = 0;
        char fileChunkFromParent[CHUNK_SIZE];
        while (counter < fileSize) {
            counter += read(parentToChildAPipe[0], fileChunkFromParent, sizeof(fileChunkFromParent));
            strcat(fileContent, fileChunkFromParent);
        }
        close(parentToChildAPipe[0]); // Close read end of parent -> child pipe

        int sharedMemoryAFD;
        void *sharedMemAPtr;
        sharedMemoryAFD = shm_open("sharedMemoryA", O_CREAT | O_RDWR, 0666);
        ftruncate(sharedMemoryAFD, fileSize);
        sharedMemAPtr = mmap(NULL, fileSize, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryAFD, 0);

        writeToSharedMemory(sharedMemAPtr, fileContent);

        close(parentToChildAPipe[0]); // Close read end of parent -> child pipe
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
        if (childBPID == 0) {
            // child b will run this part
            close(parentToChildBPipe[1]); // Close write end of parent -> child pipe
            close(childBToParentPipe[0]); // Close read end of childB -> parent pipe
            // close child a related pipes
            close(childAToParentPipe[0]);
            close(childAToParentPipe[1]);
            close(parentToChildAPipe[0]);
            close(parentToChildAPipe[1]);

            long fileSize;
            read(parentToChildBPipe[0], &fileSize, sizeof(fileSize));

            char *fileContent = malloc( sizeof(char) * ( fileSize + 1 ) );
            char fileChunkFromParent[CHUNK_SIZE];
            long counter = 0;
            while (counter < fileSize) {
                counter += read(parentToChildBPipe[0], fileChunkFromParent, sizeof(fileChunkFromParent));
                strcat(fileContent, fileChunkFromParent);
            }
            close(parentToChildBPipe[0]); // Close read end of parent -> child pipe

            // convert the file to hex
            char *hexFile = malloc( sizeof(char) * ( 2 * fileSize + 1 ) );
            strToHEXStr(fileContent, hexFile);

            // create shared memory with parent
            int sharedMemoryBFD;
            void *sharedMemBPtr;
            sharedMemoryBFD = shm_open("sharedMemoryB", O_CREAT | O_RDWR, 0666);
            ftruncate(sharedMemoryBFD, fileSize * 2);
            sharedMemBPtr = mmap(NULL, fileSize * 2, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryBFD, 0);

            writeToSharedMemory(sharedMemBPtr, hexFile);
            write(childBToParentPipe[1], doneMessage, sizeof(doneMessage));
            close(childBToParentPipe[1]);
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

            // get the file name
            char fileName[MAX_FILE_PATH_LENGTH];
            scanf("%s", fileName);

            // open the file
            FILE *filePtr;
            filePtr = fopen(fileName, "r");
            if (filePtr == NULL) {
                perror("Could not open input file");
                return -1;
            }

            // learn the file size
            long fileSize;
            fseek(filePtr, 0L, SEEK_END);
            fileSize = ftell(filePtr);
            rewind(filePtr);

            // read the file content
            char *fileContent = malloc( sizeof(char) * ( fileSize + 1 ) );
            char contentBuffer[CHUNK_SIZE];
            while (fgets(contentBuffer, CHUNK_SIZE, filePtr)) {
                strcat(fileContent, contentBuffer);
            }

            // create shared memory with childA
            int sharedMemoryAFD;
            void *sharedMemAPtr;
            sharedMemoryAFD = shm_open("sharedMemoryA", O_CREAT | O_RDWR, 0666);
            ftruncate(sharedMemoryAFD, fileSize);
            sharedMemAPtr = mmap(NULL, fileSize, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryAFD, 0);

            // create shared memory with childB
            int sharedMemoryBFD;
            void *sharedMemBPtr;
            sharedMemoryBFD = shm_open("sharedMemoryB", O_CREAT | O_RDWR, 0666);
            ftruncate(sharedMemoryBFD, fileSize * 2);
            sharedMemBPtr = mmap(NULL, fileSize * 2, PROT_WRITE|PROT_READ, MAP_SHARED, sharedMemoryBFD, 0);

            // send the fileSize to children using pipes
            write(parentToChildAPipe[1], &fileSize, sizeof(fileSize));
            write(parentToChildBPipe[1], &fileSize, sizeof(fileSize));

            // send file contents to children as chunks
            long offset = 0;
            while (offset < fileSize) {
                write(parentToChildAPipe[1], (fileContent + offset), CHUNK_SIZE);
                write(parentToChildBPipe[1], (fileContent + offset), CHUNK_SIZE);
                offset += CHUNK_SIZE;
            }
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