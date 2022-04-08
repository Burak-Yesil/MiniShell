#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>

int cd(char *path)
{
    char *token = "";
    char *s = "/";
    char cwd[PATH_MAX];
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL)
    {
        fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        return 1;
    }
    const char *home = pw->pw_dir;
    if (strcmp(path, "cd") == 0)
    {
        chdir(home);
        return 0;
    }
    if (getcwd(cwd, 256 * sizeof(char)) == NULL)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        return 1;
    }

    // loop start
    token = strtok(path, "/");
    while (token != NULL)
    {
        if (strcmp(token, "~") == 0)
        {

            chdir(home);
            strcpy(cwd, home);
            token = strtok(NULL, "/");
        }
        else if (strcmp(token, "home") == 0)
        {
            chdir(home);
            strcpy(cwd, home);
            token = strtok(NULL, "/");
        }
        else
        {
            strcat(cwd, "/");
            strcat(cwd, token);
            if (chdir(cwd) == -1)
            {
                fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", path, strerror(errno));
                return 1;
            }
            token = strtok(NULL, "/");
        }
    }
    return 0;
}

int parse(char *argv[], int argc)
{
    struct stat stats;
    // insert if-block checking what command it is
    if (strcmp(argv[0], "exit") == 0)
    {
        if (argc >= 2)
        {
            fprintf(stderr, "Error: Too many args for exit. %s.\n", strerror(errno));
            return 1;
        }
        return 2;
    }
    if (strcmp(argv[0], "cd") == 0)
    {
        if (argc == 1)
        {
            cd(argv[0]);
            return 0;
        }
        if (argc > 2)
        {
            fprintf(stderr, "Error: Too many args for cd. %s.\n", strerror(errno));
            return 1;
        }
        return cd(argv[1]);
    }

    else
    { //

        pid_t pid = fork();
        argv[argc + 1] = NULL;
        argc++;
            
        
        if (pid < 0)
        {
            fprintf(stderr, "Error: fork() failed. %s.\n" , strerror(errno));
            return 1;
        }
        wait(NULL);
        if (pid == 0)
        {
            if (execvp(argv[0], argv) < 0){
                 fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                 return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    char command[1024];
    char *argvi[1024];
    char line[1024];
    char *token = " ";
    int argci = 0;
    struct passwd *pwde;
    char *user;
    char cwd[PATH_MAX];
    uid_t uid = getuid();
    if ((pwde = getpwuid(getuid())) == NULL)
    {
        fprintf(stderr, "Error: Cannot get password entry. %s.\n", strerror(errno));
        return 1;
    }
    user = pwde->pw_name;
    if (getcwd(cwd, 256 * sizeof(char)) == NULL)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        return 1;
    }
    int parseresult = 0; // stores result of 'parse'
    while (1)
    {
        if (getcwd(cwd, 256 * sizeof(char)) == NULL)
        {
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
            return 1;
        }
        printf("Msh:%s:%s>", user, cwd);
        if ((fgets(command, 1024, stdin) == NULL) && ferror(stdin))
        {
            fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
        }
        command[strlen(command) - 1] = '\0';
        if (!(command[0] == ' '))
        {
            token = strtok(command, " ");
            while (token != NULL)
            {
                argvi[argci] = token;
                argci++;
                token = strtok(NULL, " ");
            }
            parseresult = parse(argvi, argci);
            if (parseresult == 1)
            {
                // invalid command
            }
            else if (parseresult == 2)
            {
                // wait here for children to finish if there are any running.
                exit(0);
            }
        }
        argci = 0;
        memset(argvi, 0, 1024);
        // end of loop
    }
    return 0;
}