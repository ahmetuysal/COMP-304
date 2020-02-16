#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

int main(void) {
    int parentToChildPipe[2];
    if (pipe (parentToChildPipe))
    {
        perror("Parent -> Child Pipe failed");
    }

    int childToParentPipe[2];
    if (pipe (childToParentPipe))
    {
        perror("Child -> Parent Pipe failed");
    }

    pid_t childPID = fork();

    if (childPID == 0) {
        // child will run this part
        close(parentToChildPipe[1]); // Close write end of parent -> child pipe
        close(childToParentPipe[0]); // Close read end of child -> parent pipe

        int messageFromParent;
        // read the message from parent
        read(parentToChildPipe[0], &messageFromParent, sizeof(messageFromParent));
        close(parentToChildPipe[0]); // Close read end of parent -> child pipe since we no longer need it

        int childResult = 2 * messageFromParent;
        printf("Child: Input %d, Output %d\n", messageFromParent, childResult);

        int childToGrandchildPipe[2];
        if (pipe (childToGrandchildPipe))
        {
            perror("Child -> Grandchild Pipe failed");
        }
        int grandchildToChildPipe[2];
        if (pipe (grandchildToChildPipe))
        {
            perror("Grandchild -> Child Pipe failed");
        }

        pid_t grandchildPID = fork();
        if (grandchildPID == 0) {
            // grandchild will run this part
            close(childToGrandchildPipe[1]); // Close write end of child -> grandchild pipe
            close(grandchildToChildPipe[0]); // Close read end of grandchild -> child pipe

            int messageFromChild;
            // read the message from parent
            read(childToGrandchildPipe[0], &messageFromChild, sizeof(messageFromChild));
            close(childToGrandchildPipe[0]); // Close read end of child -> grandchild pipe since we no longer need it
            int grandchildResult = 2 * messageFromChild;
            printf("Grandchild: Input %d, Output %d\n", messageFromChild, grandchildResult);

            write(grandchildToChildPipe[1], &grandchildResult, sizeof(grandchildResult));
            close(grandchildToChildPipe[1]); // close write end of grandchild -> child pipe since we no longer need it
            // Note: grandchild terminates itself after this point since main function returns
        }
        else if (childPID < 0) {
            /* The fork failed. */
            perror("Fork failed");
        }
        else {
            // child will run this part
            close(childToGrandchildPipe[0]); // Close read end of child -> grandchild pipe
            close(grandchildToChildPipe[1]); // Close write end of grandchild -> child pipe

            write(childToGrandchildPipe[1], &childResult, sizeof(childResult));
            close(childToGrandchildPipe[1]); // Close write end of child -> grandchild pipe since we no longer need it

            // read the result from grandchild
            int messageFromGrandchild;
            read(grandchildToChildPipe[0], &messageFromGrandchild, sizeof(messageFromGrandchild));
            close(grandchildToChildPipe[0]); // close read end of grandchild -> child pipe since we no longer need it

            // send it back to parent
            write(childToParentPipe[1], &messageFromGrandchild, sizeof(messageFromGrandchild));
            close(childToParentPipe[1]); // close write end of child -> parent pipe since we no longer need it
        }
    }
    else if (childPID < 0) {
        /* The fork failed. */
        perror("Fork failed");
    }
    else {
        // parent will run this part
        close(parentToChildPipe[0]); // Close read end of parent -> child pipe
        close(childToParentPipe[1]); // Close write end of child -> parent pipe

        int number;
        printf("Enter an integer: ");
        scanf("%d", &number);
        write(parentToChildPipe[1], &number, sizeof(number));
        close(parentToChildPipe[1]); // Close write end of parent -> child pipe since it is no longer needed

        int messageFromChild;
        // read the message from child
        read(childToParentPipe[0], &messageFromChild, sizeof(messageFromChild));
        close(childToParentPipe[0]); // Close read end of child -> parent pipe since it is no longer needed

        printf("Parent: Final result is %d.\n", messageFromChild);
        // kill the child and grandchild process
        int r;
        r = kill(childPID, SIGKILL);
        if (r == 0) {
            printf("Child %d killed\n", childPID);
        }
        else {
            perror("Kill Child failed");
        }
        // Grandchild is already dead since it returned after sending final result to child
        r = wait(NULL);
        if (r != childPID) {
            perror("Wait failed");
        }
    }
    return  0;
}