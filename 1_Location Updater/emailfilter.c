#include<stdio.h>
#include<time.h>
#include<string.h>
#define MAX 100


void main(int argc, char* argv[]){
    // read strings from the standard input
    char email[MAX];
    char calendar[MAX][MAX];
    int i,n = 0,count=0; //n is used to get the return value of sscanf;
    char op; // a single character: C,D,X
    char title[10], date[10], tim[5],location[10];
    struct tm t = {};
    //printf("Enter emails:\n");
    while(fgets(email,MAX,stdin)!=NULL){
        email[strcspn(email,"\r\n")]=0;   
        //Verify if the input is a calender relevant event
        //n = sscanf(email,"Subject: %c,%10[0-9a-zA-Z!-/ ],%10[0-9/ ],%5[0-9: ],%10[0-9a-zA-Z!-/ ]",&op,title,date,tim,location);
        n = sscanf(email,"Subject: %c,%10[^,],%10[^,],%5[^,],%10s",&op,title,date,tim,location);
        //printf("%s\n",email);
        //printf("%c\n%s\n%s\n%s\n%s\n",op,title,date,tim,location);
        if(n==5){
            //Step1: op
            if(op=='C' || op=='D' || op=='X'){
                //step2: date and time
                if(sscanf(date,"%d/%d/%d",&t.tm_mon,&t.tm_mday,&t.tm_year)==3){
                    if(sscanf(tim,"%d:%d", &t.tm_hour,&t.tm_min)==2){
                        if(mktime(&t)!=-1)
                            //strcpy(calendar[count++],email);
                            printf("%s\n",&email[9]);
                    }
                }   
            }
        }
        //else, read the next line
    }
}
