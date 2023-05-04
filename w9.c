#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>

int pid_child;
    int pid;
    int i=0;
    int status;
    char buff[100];
    char buffP[100];

int main(int argc, char* argv[]){


    for(i=0;i<4;i++){

        if((pid=fork())<0){

            perror("err");
        }
        if((pid==0)){

            sprintf(buff,"process %d with pid %d and parrent %d\n",i,getpid(),getppid());
            printf("%s",buff);
            exit(i);
        }
        sleep(1);
    }


    sprintf(buffP,"Parent process with pid %d and parrent %d\n",getpid(),getppid());
        printf("%s",buffP);

    for(i=0;i<4;i++){

        pid_child=wait(&status);
        if(pid_child<0){

            perror("wait err");
        }

        if((WIFEXITED(status))){

            printf("process with pid%d exited with status%d\n",pid_child,WEXITSTATUS(status));
        }
    }
}