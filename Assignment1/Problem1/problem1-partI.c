#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

int main(void) {
    pid_t childPID = fork();
    if (childPID == 0) {
        // child will run this part
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
    }
    else {
        // parent will run this part
        sleep(5);
        // kill the child process
        int r;
        r = kill(childPID, SIGKILL);
        if (r == 0) {
            printf("Child %d killed\n", childPID);
        }
        else {
            perror("Kill failed");
        }
        r = wait(NULL);
        if (r != childPID) {
            perror("Wait failed");
        }
    }
    return  0;
}