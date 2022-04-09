#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <limits.h>



int cd(char *path)
{
    struct passwd *pw;
    char *token = "";
    char cwd[PATH_MAX];

    if ((pw = getpwuid(getuid())) == NULL)
    {
        fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        return 1;
    }
    
    
    const char *homeDir = pw->pw_dir;
    if (strcmp(path, "cd") == 0)
    {
        chdir(homeDir);
        return 0;
    }
    if (getcwd(cwd, 2048) == NULL)
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
            chdir(homeDir);
            strcpy(cwd, homeDir);
            token = strtok(NULL, "/");
        }
        else if (strcmp(token, "home") == 0)
        {
            chdir(homeDir);
            strcpy(cwd, homeDir);
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

int command_parser(char *argv[], int argc, char *command, char *user, char*cwd)
{
    // Executes exit command if true
    
    if (strcmp(argv[0], "exit") == 0)
    {
        if (argc >= 2)
        {
            fprintf(stderr, "Error: Too many args for exit.\n");
            return 0;
        }
        return 2; // symbol for exit
    }

    // Executes cd command if true
    if (strcmp(argv[0], "cd") == 0)
    {
        if (argc == 1)
        {
            cd(argv[0]);
            return 0;
        }
        if (argc > 2)
        {
            fprintf(stderr, "Error: Too many args for cd.\n");
            return 0;
        }
        if (strcmp(argv[argc - 1], "&") == 0)
        {
            fprintf(stderr, "Error: cd cannot run in background.\n");
        }
        return cd(argv[1]);
    }

    // executes built in command if true
    if (strcmp(argv[argc - 1], "&") == 0)
    { // background process
        pid_t pid = fork();

        if (pid == -1)
        {
            fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
            return 1;
        }
        
        
        
        else if (pid == 0)
        {
            printf("pid: %d cmd: %s", getpid(), command);
            argv[argc - 1] = '\0'; // remove &
            if ((execvp(argv[0], argv) == -1))
            {
                fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                return 1;
            }
            return 0;
        }
        else if (pid > 0)
        {
            while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
            {
                printf("pid %d done. Click Enter to Continue\n", getpid());
            }
            //printf("Msh:%s:%s>", user, cwd);
        }
    }
    else
    { // foreground process
        pid_t pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
            return 1;
        }
        else if (pid == 0)
        {
            if ((execvp(argv[0], argv) == -1))
            {
                fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                return 1;
            }
            return 0;
        }
        else if (pid > 0)
        {
            int wait;
            if (waitpid(pid, &wait, 0) == -1)
            {
                fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                return 1;
            }
        }
    }
    return 0;
}


void bg_helper(int signal)
{
    int temperrno = errno;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("\npid %d done.\n", pid);
    }
    errno = temperrno;
}

void sigint_helper(int sign)
{
    signal(sign, SIG_IGN);
    fprintf(stderr, "\nError: Only exit can terminate the shell.\n");
}




int main(int argc, char *argv[])
{
    int MaxLine = 1024;
    char command[MaxLine];
    char *argvi[MaxLine];
    char *token = " ";
    int argci = 0;
    struct passwd *pwde;
    char *user;
    char cwd[PATH_MAX];
    if (signal(SIGINT, sigint_helper) == SIG_ERR)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }
    if (signal(SIGCHLD, bg_helper) == SIG_ERR)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }
    if ((pwde = getpwuid(getuid())) == NULL)
    {
        fprintf(stderr, "Error: Cannot get password entry. %s.\n", strerror(errno));
        return 1;
    }
    if (getcwd(cwd, 2048) == NULL)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        return 1;
    }

    int parseresult = 0; // stores result of 'parse'
    user = pwde->pw_name;

    while (1)
    {
        if (getcwd(cwd, 2048) == NULL)
        {
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
            return 1;
        }
        printf("Msh:%s:%s>", user, cwd);
        if ((fgets(command, MaxLine, stdin) == NULL) && ferror(stdin))
        {
            fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
        }
        
        if (strcmp(command, "\n") == 0){
        	continue;
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
            parseresult = command_parser(argvi, argci, command, user, cwd);
        
            if (parseresult == 2)
            {
                break; //Exit command called so exiting the loop
            }
        }
        // reset argc and argv
        argci = 0;
        memset(argvi, 0, MaxLine);
        // end of loop
    }
    if (strcmp(argvi[0], "exit") != 0)
    {
        killpg(getpid(), SIGTERM);
    }
    return 0;
}