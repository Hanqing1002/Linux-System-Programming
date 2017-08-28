#include<stdio.h>
#include<time.h>
#include<string.h>
#define MAX 100

struct calendar{
    char title[10];
    char date[10];
    char time[5]; 
    char location[10];
};

void main(int argc, char* argv){
    char event[MAX]; // input buffer
    struct calendar calendars[MAX];
    int i,j,count,flag=0,n;
    char op; // a single character: C,D,X
    char tit[10], d[10], tim[5],l[10];

    //printf("Enter calendar events:\n");
    while(fgets(event,MAX,stdin)!=NULL){
        // reformat the input string
        n = sscanf(event,"%c,%10[^,],%10[^,],%5[^,],%10s",&op,tit,d,tim,l);
        //n = sscanf(event,"%c",&op);
        //printf("%s\n",event);
        //printf("%c\n",op);
        //printf("%c\n%s\n%s\n%s\n%s\n",op,tit,d,tim,l);
        tit[10] = '\0';
	    d[10] = '\0';
        tim[5] = '\0';
        l[10] = '\0';
        if(n==5){
            switch(op){
                case 'C'://add a new entry in the calendars array
                    strcpy(calendars[count].title,tit);
                    //printf("%s\n",calendars[count].title);
                    strcpy(calendars[count].date,d);
                    //printf("%s\n",calendars[count].date);
                    strcpy(calendars[count].time,tim);
                    //printf("%s\n",calendars[count].time);
                    strcpy(calendars[count].location,l);
                    //printf("%s\n",calendars[count].location);
                    //printf("%s\n%s\n%s\n%s\n",calendars[count].title,calendars[count].date,calendars[count].time,calendars[count].location);
                    //Check if this new event is the earlist one
                    for(i=0;i<=count;i++){
                        flag=0;
                        if(strncmp(calendars[i].date,calendars[count].date,10)==0
                            && strncmp(calendars[i].time,calendars[count].time,5)<0){
                            flag = 1;break;}
                    }
                    if(flag==0)                 
                        printf("%-10.10s:%-10.10s\n", calendars[count].date,calendars[count].location);
                    count++;                    
                    break;
                case 'D': //empty the deleted event
                    if(count!=0){
                        for(i=0;i<count;i++){
                            //printf("%s\n%s\n",calendars[i].title,tit);
                            //printf("%10.10s\n",calendars[i].title);
                            if(strncmp(calendars[i].title,tit,10)==0
                                    && strncmp(calendars[i].date,d,10)==0
                                    && strncmp(calendars[i].time,tim,5)==0
                                    && strncmp(calendars[i].location,l,10)==0){
                            
                                //printf("%d\n",i); 
                                strncpy(calendars[i].location,"None      ",10);
                                //printf("%10.10s\n",calendars[i].location);
                                break;
                            }
                        }
                        for(j=0;j<=count;j++){
                            flag=0;
                            if(strncmp(calendars[j].date,calendars[i].date,10)==0
                                && strncmp(calendars[j].time,calendars[i].time,5)>0){
                                flag = 1;break;}
                        }
                        if(flag==0)                   
                            printf("%-10.10s:%-10.10s\n", calendars[i].date,calendars[i].location);
                    }
                    break;
                case 'X': //update the change
                    if(count!=0){
                        char pre_time[5];
                        for(i=0;i<count;i++){
                            //printf("%s\n%s\n",calendars[i].title,tit);
                            if(strncmp(calendars[i].title,tit,10)==0
                                    && strncmp(calendars[i].date,d,10)==0
                                    && strncmp(calendars[i].location,"None      ",10)!=0){
                                //printf("%d\n",i); 
                                strncpy(pre_time,calendars[i].time,10);
                                strncpy(calendars[i].time,tim,5);
                                strncpy(calendars[i].location,l,10);
                                break;
                            }
                        }
                        for(j=0;j<=count;j++){
                            flag=0;
                            if(strncmp(calendars[j].date,calendars[i].date,10)==0
                                && strncmp(calendars[j].time,pre_time,5)<0){
                                flag = 1;
                                break;}
                        }
                        if(flag==0)                   
                            printf("%-10.10s:%-10.10s\n", calendars[i].date,calendars[i].location);
                    }
                    break;
            }
        }
    }
}
