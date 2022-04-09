#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <sys/errno.h>
#include "minishell.h"


//cd function changes directory
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
    const char *homeDir = pw->pw_dir; //getting path to directory
    if (strcmp(path, "cd") == 0) //If no args just go to home
    {
        chdir(homeDir);
        return 0;
    }
    if (getcwd(cwd, 2048) == NULL)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        return 1;
    }
    token = strtok(path, "/"); //Tokenizing the path parameter
    while (token != NULL)//Going through a loop while updating the current directory to end up at desired directory
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



//command_parser parses and runs the commands 
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
        return 2; // Value used to let main function now to exit the while loop
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

 
    //Executes other processes 

    if (strcmp(argv[argc - 1], "&") == 0) //If command has an & it should run in the background
    { 
        pid_t pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
            return 1;
        }
        else if (pid == 0)
        {
            argv[argc - 1] = '\0'; 
            printf("pid: %d cmd: %s", getpid(), command); //should print out the current running process
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
    else //otherwise the command should run in the foreground
    { 
        pid_t pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
            return 1;
        }
        else if (pid == 0)
        {
            if ((execvp(argv[0], argv) == -1)) //replaces the current child process with inputed command process
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


//Signal Helpers used in main function
void bg_helper(int signal)
{
    int error_code = errno;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("\npid %d done.\n", pid);
    }
    errno = error_code;
}

void sigint_helper(int sign)
{
    signal(sign, SIG_IGN);
    fprintf(stderr, "\nError: Only exit can terminate the shell.\n");
}



//Main Function
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


    //Checking and setting signals
    if (signal(SIGCHLD, bg_helper) == SIG_ERR)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }

    if (signal(SIGINT, sigint_helper) == SIG_ERR)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }

    //code to get the prompt data to use in the prompt
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

    int count= 0; 
    user = pwde->pw_name;

    //Infinite loop keeps on showing prompt and handles commands in foreground and background
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
        
        //Making sure to redisplay prompt if nothing is typed and enter is clicked
        //This prevents segfaults from occuring
        if (strcmp(command, "\n") == 0){
        	continue;
        }
        
        
        command[strlen(command) - 1] = '\0';
        
      
        if (!(command[0] == ' '))
        {   
            //Tokenizing command 
            token = strtok(command, " ");
            while (token != NULL)
            {
                argvi[argci] = token;
                argci++;
                token = strtok(NULL, " ");
            }
            count = command_parser(argvi, argci, command, user, cwd);
        
            if (count == 2)
            {
                break; //Exit command called so exiting the loop
            }
        }
        argci = 0;
        memset(argvi, 0, MaxLine);
    }

    //Killing all children processes
    if (strcmp(argvi[0], "exit") != 0)
    {
        killpg(getpid(), SIGTERM);
    }
    return 0;
}