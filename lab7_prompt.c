#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h>
#include<sys/wait.h>
#include <pwd.h>
#include<string.h>
#include<sys/errno.h>
#include <limits.h>


int printPrompt(){
        struct passwd *p;
        uid_t uid;
        char cwd[PATH_MAX];

        if((p=getpwuid(uid = getuid())) == NULL){
            perror("getpwuid() error");
            return 1;
        }else{

            if(getcwd(cwd, sizeof(cwd)) == NULL){
                perror("getcwd() error");
                return 1;
            }
            printf("MSh:%s:%s>",p->pw_name, cwd);
        }
}


int main()
{
  int MaxLine = 1024;
    char command[MaxLine];
    chdir("/home");
    
    while(1){

	//Printing prompt and getting user typed input 
        printPrompt();

        if ((fgets(command, MaxLine, stdin) == NULL) && ferror(stdin)){
            perror("invalid command");
        }

        if(feof(stdin)){
            printf("\n");
            exit(0);
        }
        
        if(strcmp("exit\n", command) == 0){
		return EXIT_FAILURE;
        }
        
        char *tokens = strtok(command, " ");
        
       if(strcmp(tokens[0], "cd") == 0){
       	strcat("/",tokens[1]);
       	printf("%s", tokens[1]);
       	//chdir();
       }
    }

    /*
      TODO:
      1. Use getuid, getpwuid, getcwd to retrieve the username and home 
      directory of the current user
      2. Change the directory to the user's home directory
      3. Print the prompt as specified in homework 5 and wait for user input
    */

    /* 
      TODO:
      1. Handle cd commands entered by the user. Check that a single argument 
      is provided. Ignore other commands, but don't exit. Change the current 
      directory according to the user's input.
      2. Handle the exit command. Ensure that it is followed by no arguments.

      ** If tokenizing is giving you trouble, allow the user to omit cd and
      consider the received string as the target directory. There should be 
      no directory named `exit'
    */
    
    return EXIT_SUCCESS;
}
