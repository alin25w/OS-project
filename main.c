#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>


#define MAX_PATH_LENGTH 256 

void counter_line(char *filename){
   
    FILE *fp;
    char c;
    int lines = 0;

    fp = fopen(filename, "r");
    if (fp == NULL){
      
        printf("Can't open file %s\n", filename);
        exit(1);
    }

    while ((c = getc(fp)) != EOF){
        
        if (c == '\n'){

            lines++;
        }
    }

    printf("File %s has %d lines\n", filename, lines);
    fclose(fp);
}

int CheckCFile(char *filepath){

    //reach .something and check if .c
    char *extp = strrchr(filepath,'.');
    if(extp && !strcmp(extp,".c")){
        
        return 1;
    }
    return 0;
}

void PrintAccessRights(mode_t mode){
   
    printf("User:\n");
    printf("Read - %s\n",(mode & S_IRUSR) ? "yes" : "no");
    printf("Write - %s\n",(mode & S_IWUSR) ? "yes" : "no");
    printf("Exec - %s\n",(mode & S_IXUSR) ? "yes" : "no");
    printf("Group:\n");
    printf("Read - %s\n",(mode & S_IRGRP) ? "yes" : "no");
    printf("Write - %s\n",(mode & S_IWGRP) ? "yes" : "no");
    printf("Exec - %s\n",(mode & S_IXGRP) ? "yes" : "no");
    printf("Others:\n");
    printf("Read - %s\n",(mode & S_IROTH) ? "yes" : "no");
    printf("Write - %s\n",(mode & S_IWOTH) ? "yes" : "no");
    printf("Exec - %s\n",(mode & S_IXOTH) ? "yes" : "no");
}

void CreateSymbolicLink(char *filepath){
    
    char linkname[MAX_PATH_LENGTH];
    printf("Please give the link name: ");
    scanf("%s", linkname);
    if (symlink(filepath, linkname) == -1){

        perror("Error creating symbolic link");
    }
    else{

        printf("Symbolic link created successfully.\n");
    }
}

void ExecuteRegularFileOption(char option, char *filepath){

    struct stat sb;
    if (stat(filepath, &sb) == -1) {

        perror("Failed to get file information");
    }
    switch (option){

    case 'n':
        printf("Name (-n): %s\n",filepath);
        break;
    case 'd':
        printf("Size (-d): %ld bytes\n",sb.st_size);
        break;
    case 'h':
        printf("Hard link count (-h): %ld\n",sb.st_nlink);
        break;
    case 'm':
        printf("Last modified on: (-m): %s",ctime(&sb.st_mtime));
        break;
    case 'a':
        printf("Access rights (-a):\n");
        PrintAccessRights(sb.st_mode);
        break;
    case 'l':
        printf("Create symbolic link (-l): ");
        CreateSymbolicLink(filepath);
        break;
    default:
        break;
    }
}

void ExecuteDirectoryOption(char option, char *filepath){

    struct dirent *dp;
    struct stat sb;
    DIR *dir = opendir(filepath);
    int countC = 0;

    if (!dir){

        perror("Can't open directory.");
        return;
    }

    //parse directory

    while ((dp = readdir(dir)) != NULL){

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", filepath, dp->d_name);

        if (lstat(full_path, &sb) == -1){

            perror("Can't get file statistics for file in directory.");
            continue;
        }

        switch (option){

        case 'n':
            printf("%s\n", dp->d_name);
            break;

        case 'd':
            printf("%ld\n", sb.st_size);
            break;

        case 'a':
            PrintAccessRights(sb.st_mode);
            break;

        case 'c':
            if (S_ISREG(sb.st_mode) && strstr(dp->d_name,".c")!=NULL){

                countC++;
            }
            break;

        default:
            printf("Bad format: %c\n", option);
            break;
        }
    }

    if (option == 'c'){

        printf("Total C files: %d\n", countC);
    }

    closedir(dir);
}

void ExecuteSymbolicLinkOption(char option, char *filepath){

    struct stat sb;
    char target_path[MAX_PATH_LENGTH];
    ssize_t target_size;
    int delete_flag = 0;

    if (lstat(filepath, &sb) == -1){

        perror("lstat");
        return;
    }

    if (S_ISLNK(sb.st_mode)){

        //target path and size

        target_size = readlink(filepath, target_path, MAX_PATH_LENGTH - 1);
        if (target_size == -1){

            perror("readlink");
            return;
        }
        target_path[target_size] = '\0';
    }

    switch (option){

    case 'n':
        //name
        printf("%s\n", filepath);
        break;

    case 'l':
        //delete symlink
        if (unlink(filepath) == -1){

            perror("unlink");
        }
        delete_flag = 1;
        break;

    case 'd':
        //size of link
        printf("%ld\n", target_size);
        break;

    case 't':
        //size of target
        printf("%ld\n", sb.st_size);
        break;

    case 'a':
        PrintAccessRights(sb.st_mode);
        break;

    default:
        printf("Bad format: %c\n", option);
        break;
    }

}

void DisplayRegularFileMenu(char *filepath){

    printf("Options:\n");
    printf("-n: name\n");
    printf("-h: hard link count\n");
    printf("-d: file size\n");
    printf("-m: time of last modification\n");
    printf("-a: access rights\n");
    printf("-l: create symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): \n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // make \n \0 so we don't get stuck
    int i;
    int check = 1;
    char option;

    //validate options
    for (i = 1; i < strlen(options); i++){

        option = options[i];
        if (!(strchr("nhdmal", option))){

            check = 0;
            break;
        }
    }

    if (check == 1){

        for (i = 1; i < strlen(options); i++){

            option = options[i];
            ExecuteRegularFileOption(option, filepath);
        }
    }
    else{

        perror("Bad format");
        DisplayRegularFileMenu(filepath);
        return;
    }
}

void DisplayDirectoryMenu(char *filepath){

    printf("Options:\n");
    printf("-n: name of file\n");
    printf("-d: file size\n");
    printf("-a: access rights\n");
    printf("-c: total number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): \n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // make \n \0 so we don't get stuck
    int i;
    int check = 1;
    char option;

    //validate options
    for (i = 1; i < strlen(options); i++){

        option = options[i];
        if (!(strchr("ndac", option))){

            check = 0;
            break;
        }
    }

    if (check == 1){

        for (i = 1; i < strlen(options); i++){

            option = options[i];
            ExecuteDirectoryOption(option, filepath);
        }
    }
    else{

        perror("Bad format");
        DisplayDirectoryMenu(filepath);
        return;
    }
}

void DisplaySymbolicLinkMenu(char *filepath){

    printf("Options:\n");
    printf("-n: name of file\n");
    printf("-d: symbolic link size\n");
    printf("-a: access rights\n");
    printf("-t: size of target file\n");
    printf("-l: delete symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): \n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // make \n \0 so we don't get stuck
    int i;
    int check = 1;
    char option;

    //validate options
    for (i = 1; i < strlen(options); i++){

        option = options[i];
        if (!(strchr("nldta", option))){

            check = 0;
            break;
        }
    }

    if (check == 1){

        for (i = 1; i < strlen(options); i++){

            option = options[i];
            ExecuteSymbolicLinkOption(option, filepath);
        }
    }
    else{

        perror("Bad format");
        DisplaySymbolicLinkMenu(filepath);
        return;
    }
}


void DisplayFileInfo(char *filepath){

    struct stat sb;
    if (stat(filepath, &sb) == -1){

        perror("Failed to get file information");
        return;
    }
    switch (sb.st_mode & S_IFMT){

    case S_IFREG:
        printf("File type: regular file\n");
        DisplayRegularFileMenu(filepath);
        break;
    case S_IFDIR:
        printf("File type: directory\n");
        DisplayDirectoryMenu(filepath);
        break;
    case S_IFLNK:
        printf("File type: symbolic link\n");
        DisplaySymbolicLinkMenu(filepath);
        break;
    default:
        printf("File type: unknown\n");
    }
}

void ExecuteScript(char *filepath,int pfd[2]){

    char command[100];
    //close unused pipe
    close(pfd[0]);
    sprintf(command, "bash script.sh %s", filepath);

    FILE *pipe;
    char output[20];

    pipe = popen(command, "r");
    if (pipe == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    while (fgets(output, sizeof(output), pipe)) {
        write(pfd[1], output, strlen(output));
    }

    pclose(pipe);
	close(pfd[1]);
}

void FirstChildProcess(char *filepath){

    DisplayFileInfo(filepath);
}

//if arg is symlink
void ChangeLinkAccessRights(char *filepath){
    char command[100];
    sprintf(command,"chmod 760 %s",filepath); 
    system(command);
}

//if arg is dir
void CreateTextFile(char *filepath){

    char *name = strrchr(filepath,'/');
    char command[100];
    
    sprintf(command,"touch %sFILE.txt",filepath);
    system(command);
}

//if arg is C file
void ErrorsWarnings(char buff[5],char *filepath){
    int errors = -1, warnings = -1, score;
    FILE *grades = fopen("grades.txt", "a");

    sscanf(buff, "%d %d", &errors, &warnings);
    if(errors > 0)
        score = 1;
    else if(errors == 0){
        if(warnings == 0)
            score = 10;
        else if(warnings > 10)
                score = 2;
            else if(warnings < 10) 
                    score = 2 + 8 *(10 - warnings)/10;
    }
    
        if (grades == NULL){
            perror("Error at fopen grades");
            return;
        }
        fprintf(grades, "%s: Score%d\n with %d errors and %d warnings",filepath,score,errors,warnings);
        fclose(grades);

    
}

void SecondChildProcess(char *filepath, int pfd[2]){

     struct stat sb;

     if (stat(filepath, &sb) == -1){
       
        perror("Failed to get file information");
        return;
    }

    if(S_ISREG(sb.st_mode)){
                if (strstr(filepath, ".c") != NULL){
					
                    ExecuteScript(filepath,pfd);
					exit(0);
                }
                else counter_line(filepath);
            }
            
            if(S_ISLNK(sb.st_mode)){
               
                ChangeLinkAccessRights(filepath);
            }
            
            if(S_ISDIR(sb.st_mode)){
               
                CreateTextFile(filepath);
            }
}

int main(int argc, char *argv[])
{
    struct stat file;

    if (argc < 2){

        printf("Usage: %s <file1> <file2> ...\n", argv[0]);
        exit(1);
    }

    int pid, status;

    for(int i = 1; i < argc; i++){

        int pfd[2];

        if(pipe(pfd)<0){

	        perror("Pipe creation error\n");
	    }


        if((pid = fork()) < 0){
            
            perror("Process creation error\n");
                exit(i);
        }
        else if(pid == 0){
            //child#1
            FirstChildProcess(argv[i]);
            exit(i);
            sleep(1);
        }
        //parent

        if((pid = fork()) < 0){
           
            perror("Process creation error\n");
            exit(i);
        }
        else if(pid == 0){
            //child#2
            SecondChildProcess(argv[i],pfd);
            exit(i);
        }

        //close pipes

        close(pfd[1]);

        if(CheckCFile(argv[i])){
            char buff[20];
            read(pfd[0], buff, sizeof(buff));
            ErrorsWarnings(buff,argv[i]);
        }

        close(pfd[0]);

        //wait for processes to finish and have the parent print

        sleep(1);
        int pid_Child;
        pid_Child = wait(&status);
        if(pid_Child < 0){
            
            perror("Error at pid_Child#1");
            exit(i);
        }
        //if exited normally print pid
        if(WIFEXITED(status)){
            
            printf("Process with pid %d ended with the exit code %d\n\n", pid_Child, WIFEXITED(status));
        }

        pid_Child = wait(&status);
        if(pid_Child < 0){
            
            perror("Errorr at pid_CHILD#2");
            exit(i);
        }
        //if exited normally print pid
        if(WIFEXITED(status)){
            
            printf("Process with pid %d ended with the exit code %d\n", pid_Child, WIFEXITED(status));
        }

        sleep(1);
    }
}