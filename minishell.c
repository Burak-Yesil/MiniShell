#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <stdlib.h>
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





int main(int argc, char* argv){

    int MaxLine = 1024;
    char command[MaxLine];

    while(1){

        printPrompt();

        if ((fgets(command, MaxLine, stdin) == NULL) && ferror(stdin)){
            perror("invalid command");
        }

        if(feof(stdin)){
            printf("\n");
            exit(0);
        }
        
        printf("%s", command);


        //break;
    }

    
    return 0;
}