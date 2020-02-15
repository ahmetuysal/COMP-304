#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

int main(void) {
    pid_t childPIDs[4];
    pid_t childPID;
    for (int i = 0; i < 4; ++i) {
        childPID = fork();
        if (childPID == 0) {
            child();
            break;
        }
        childPIDs[i] = childPID;
    }

    if (childPID != 0) {
        // parent will run this part
        sleep(5);
        int r;
        // kill the child processes
        for (int i = 0; i < 4; ++i) {
            r = kill(childPIDs[i], SIGKILL);
            if (r == 0) {
                printf("Child %d killed\n", childPIDs[i]);
            }
            else {
                perror("Kill failed");
            }
            r = wait(NULL);
            if (r != childPIDs[i]) {
                perror("Wait for child failed");
            }
        }
    }
    return  0;
}


int child(void) {
    struct timeval tv;
    struct tm       *tm;
    char            fmt[64], buf[64];
    pid_t myPID = getpid();
    while (1) {
        gettimeofday(&tv, NULL);
        if((tm = localtime(&tv.tv_sec)) != NULL) {
            strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%06u %z", tm);
            snprintf(buf, sizeof buf, fmt, tv.tv_usec);
            printf("My PID: %d, Current Time: %s\n", myPID, buf);
        }
        else {
            perror("Get time of day failed");
        }
        sleep(1);
    }
    return 0;
}