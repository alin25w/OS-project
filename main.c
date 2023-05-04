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

#define MAX_PATH_LENGTH 256 // buff for symbolic link

void counter_line(char *filename)
{
    FILE *fp;
    char c;
    int lines = 0;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    while ((c = getc(fp)) != EOF)
    {
        if (c == '\n')
        {
            lines++;
        }
    }

    printf("File %s has %d lines\n", filename, lines);
    fclose(fp);
}
void print_access_rights(mode_t mode)
{
    printf("User:\n");
    printf("Read - %s\n", (mode & S_IRUSR) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWUSR) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXUSR) ? "yes" : "no");
    printf("Group:\n");
    printf("Read - %s\n", (mode & S_IRGRP) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWGRP) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXGRP) ? "yes" : "no");
    printf("Others:\n");
    printf("Read - %s\n", (mode & S_IROTH) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWOTH) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXOTH) ? "yes" : "no");
}

void create_symbolic_link(char *filepath)
{
    char linkname[MAX_PATH_LENGTH];
    printf("Please give the link name: ");
    scanf("%s", linkname);
    if (symlink(filepath, linkname) == -1)
    {
        perror("Error creating symbolic link");
    }
    else
    {
        printf("Symbolic link created successfully.\n");
    }
}

void execute_regular_file_option(char option, char *path)
{
    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        perror("Failed to get file information");
    }
    switch (option)
    {
    case 'n':
        printf("Name (-n): %s\n", path);
        break;
    case 'd':
        printf("Size (-d): %ld bytes\n", sb.st_size);
        break;
    case 'h':
        printf("Hard link count (-h): %ld\n", sb.st_nlink);
        break;
    case 'm':
        printf("Time of last modification (-m): %s", ctime(&sb.st_mtime));
        break;
    case 'a':
        printf("Access rights (-a):\n");
        print_access_rights(sb.st_mode);
        break;
    case 'l':
        printf("Create symbolic link (-l): ");
        create_symbolic_link(path);
        break;
    default:
        break;
    }
}

void execute_directory_option(char option, char *path)
{
    struct dirent *dp;
    struct stat sb;
    DIR *dir = opendir(path);
    int c_count = 0;

    if (!dir)
    {
        perror("Couldn't open directory.");
        return;
    }

    while ((dp = readdir(dir)) != NULL)
    {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, dp->d_name);

        if (lstat(file_path, &sb) == -1)
        {
            perror("Couldn't get file statistics for file in directory.");
            continue;
        }

        switch (option)
        {
        case 'n':
            printf("%s\n", dp->d_name);
            break;

        case 'd':
            printf("%ld\n", sb.st_size);
            break;

        case 'a':
            print_access_rights(sb.st_mode);
            break;

        case 'c':
            if (S_ISREG(sb.st_mode) && strstr(dp->d_name, ".c") != NULL)
            {
                c_count++;
            }
            break;

        default:
            printf("Invalid option: %c\n", option);
            break;
        }
    }

    if (option == 'c')
    {
        printf("Total .c files: %d\n", c_count);
    }

    closedir(dir);
}

void execute_symbolic_link_option(char option, char *path)
{
    struct stat sb;
    char target_path[PATH_MAX];
    ssize_t target_size;
    int delete_flag = 0;

    if (lstat(path, &sb) == -1)
    {
        perror("lstat");
        return;
    }

    if (S_ISLNK(sb.st_mode))
    {
        // Get target path and size if symbolic link
        target_size = readlink(path, target_path, PATH_MAX - 1);
        if (target_size == -1)
        {
            perror("readlink");
            return;
        }
        target_path[target_size] = '\0';
    }

    switch (option)
    {
    case 'n':
        // Name of file
        printf("%s\n", path);
        break;

    case 'l':
        // Delete symbolic link
        if (unlink(path) == -1)
        {
            perror("unlink");
        }
        delete_flag = 1;
        break;

    case 'd':
        // Size of symbolic link
        printf("%ld\n", target_size);
        break;

    case 't':
        // Size of target file
        if (!S_ISLNK(sb.st_mode))
        {
            printf("Not a symbolic link\n");
            return;
        }
        if (lstat(target_path, &sb) == -1)
        {
            perror("lstat");
            return;
        }
        printf("%ld\n", sb.st_size);
        break;

    case 'a':
        // Access rights
        print_access_rights(sb.st_mode);
        break;

    default:
        printf("Invalid option: %c\n", option);
        break;
    }

    if (!delete_flag && option != '\0')
    {
        printf("Additional options not supported\n");
    }
}

void display_regular_file_menu(char *path)
{
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-h: display hard link count\n");
    printf("-d: display file size\n");
    printf("-m: display time of last modification\n");
    printf("-a: display access rights\n");
    printf("-l: create symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline character
    int i;
    int check = 1;
    char option;
    for (i = 1; i < strlen(options); i++)
    {
        option = options[i];
        if (!(strchr("nhdmal", option)))
        {
            check = 0;
            break;
        }
    }

    if (check == 1)
    {
        for (i = 1; i < strlen(options); i++)
        {
            option = options[i];
            execute_regular_file_option(option, path);
        }
    }
    else
    {
        perror("Invalid option");
        display_regular_file_menu(path);
        return;
    }
}

void display_directory_menu(char *path)
{
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display file size\n");
    printf("-a: display access rights\n");
    printf("-c: number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline character
    int i;
    int check = 1;
    char option;
    for (i = 1; i < strlen(options); i++)
    {
        option = options[i];
        if (!(strchr("ndac", option)))
        {
            check = 0;
            break;
        }
    }

    if (check == 1)
    {
        for (i = 1; i < strlen(options); i++)
        {
            option = options[i];
            execute_directory_option(option, path);
        }
    }
    else
    {
        perror("Invalid option");
        display_directory_menu(path);
        return;
    }
}

void display_symbolic_link_menu(char *path)
{
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display symbolic link size\n");
    printf("-a: display access rights\n");
    printf("-t: display size of target file\n");
    printf("-l: delete symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline character
    int i;
    int check = 1;
    char option;
    for (i = 1; i < strlen(options); i++)
    {
        option = options[i];
        if (!(strchr("nldta", option)))
        {
            check = 0;
            break;
        }
    }

    if (check == 1)
    {
        for (i = 1; i < strlen(options); i++)
        {
            option = options[i];
            execute_symbolic_link_option(option, path);
        }
    }
    else
    {
        perror("Invalid option");
        display_symbolic_link_menu(path);
        return;
    }
}


void display_file_info(char *path)
{
    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        perror("Failed to get file information");
        return;
    }
    switch (sb.st_mode & S_IFMT)
    {
    case S_IFREG: // regular file
        printf("File type: regular file\n");
        display_regular_file_menu(path);
        break;
    case S_IFDIR: // directory
        printf("File type: directory\n");
        display_directory_menu(path);
        break;
    case S_IFLNK: // symbolic link
        printf("File type: symbolic link\n");
        display_symbolic_link_menu(path);
        break;
    default:
        printf("File type: unknown\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc < 1)
    {
        printf("Usage: %s <file1> <file2> ...\n", argv[0]);
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        display_file_info(argv[i]);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("Failed fork\n");
        return 1;
    }
    else if (pid == 0)
    {
        for (int i = 1; i < argc; i++)
        {
            printf("Child with PID %d going to %d\n", getpid(), i);
        }
        return 0;
    }
    for (int i = 1; i < argc; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            printf("Failed fork\n");
            return 1;
        }
        else if (pid == 0)
        {
            counter_line(argv[i]);
        }
    }
    for (int i = 0; i < argc; i++)
    {
        sleep(1);
    }
}