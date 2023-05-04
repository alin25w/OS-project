/*
./executabil <fis.txt>

tratam orice erori prin mesaj

1 verif args minim 1
2 2 procese parinte si fiu parinte:transmite prin pipe catre fiu continutul fisiarului dat ca arg
                                    numara majuscule primite de la fiu
                                    proces fiu s-a terminat.....
                            fiu:filtreaza continut de la parinte(liniile care incep cu majuscule) si le trimite prin pipe inapoi la parinte
                            parinte si fiu ruleaza in paralel

                            fstat(argv[1]) S_ISREG
*/
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

int child[2],parent[2];
FILE* fd;
FILE* readpr;
char buff[50];

void verif_pipe(){

    if(pipe(parent)<0)
	{
		perror("Pipe creation failed\n");
	}
    if(pipe(child)<0)
	{
		perror("Pipe creation failed\n");		
	}
}

void child_(){

    if((close(parent[1]))<0){
        perror("close parent");
    }

    if((close(child[0]))<0){
        perror("close child");
    }
    dup2(parent[0],0);
    dup2(child[1],1);

    if((close(parent[0]))<0){
        perror("close parent");
    }

    if((close(child[1]))<0){
        perror("close child");
    }

    execlp("grep","grep",^[A-Z],NULL);
    printf("");
}

void parent_(char argv[]){

    if((close(parent[0]))<0){
        perror("close parent");
    }

    if((close(child[1]))<0){
        perror("close child");
    }

    if((fd=open(argv[1],O_RDONLY))){
        perror("err at open file");
    }
    while((readpr=read(fd,&buff,50)!=0)){

        if((write(parent[1],&buff,readpr))<0){
            perror("err at write");
        }
    }

    if((close(fd))<0){
        perror("err at closing file");
    }
    if((close(parent[1]))<0){
        perror("err at closing file");
    }
}

int main(int argc, char* argv[]){

   
    int pid;

    if(argc<2){
        perror("wrong nr of args");
    }

    verif_pipe();

    if((pid=fork())<0){

        perror("err at creating process");
    }

    if(pid==0){

        child_();
    }

    parent_(argv);

}