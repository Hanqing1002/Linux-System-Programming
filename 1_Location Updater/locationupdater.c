#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>
#define MAX 100
struct calendar{
    char title[10];
    char date[10];
    char time[5]; 
    char location[10];
};

int main(int argc, char* argv){
    int r0,r1,fd12[2],status0,status1;
    pipe(fd12);
    // generate 2 children
    r0 = fork();
    if(r0 > 0){
        r1 = fork();
    }
    
    if(r0>0 && r1>0){
        close(fd12[0]);
        close(fd12[1]);
        //wait
        waitpid(r0,&status0,0); 
        kill(r0,SIGKILL);
        waitpid(r1,&status1,1); 
        kill(r1,SIGKILL);
        
    }
    else if(r0==0){//P1:Email filter
        dup2(fd12[1],1); //redirect stdout to pipe12's write end
        close(fd12[0]); //close pipe12's read end
        if(execlp("./emailfilter","emailfilter",NULL)==-1)
            exit(-1);
            //close(fd12[1]);
    }
    else{//P2: calendar
        dup2(fd12[0],0); //redirect stdin to pipe12's read
        close(fd12[1]); // close pipe12's write end
        if(execlp("./calendarfilter","calendarfilter",NULL)==-1)
            exit(-1);
    }
        
    return 0;   
}

