#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<semaphore.h>

#define SIZE 100

struct Account{
    int NO;
    int balance;
};

struct Transfer{
    int from;
    int to;
    int amount;
};
struct arguments{
    int arg1;
    int arg2;
};
/*global variables*/
struct Account accounts[SIZE];
struct Transfer records[SIZE];
int cur = 0; // the next transfer waiting to be processed
pthread_mutex_t lockTransfer; // mutural exclusion for cur

pthread_mutex_t locklist[SIZE]; // one lock per account
pthread_cond_t condlist[SIZE]; // one condisional variable per account
int state[SIZE] = {0}; //0: no-op; 1: processing
pthread_mutex_t lockState; 


/*function*/
void start_transfer(int,int);
void end_transfer(int,int);
void test();
void *worker_routine(void *);



int main(int argc, char **argv){
    // check input
    if(argc!=3)
        printf("Usage of transfProg: <inputfile> <numWorkers>");
    const char* file = argv[1];
    const int numWorkers = atoi(argv[2]);
    int i= 0,numAccounts, numTransfers;
    long pos;

    /*process the file*/
    FILE *fp;
    fp = fopen(file,"r"); // read-only
    if(fp==NULL){
        fprintf(stderr,"Can't open input file!\n");
        exit(1);
    }
    //read and initialize variables
    pos = ftell(fp);
    char line[SIZE];
    while(fgets(line,sizeof(line),fp)){
        if(sscanf(line, "%d %d", &(accounts[i].NO),&(accounts[i].balance)) == 2){
            i++;
            pos = ftell(fp);
        }
        else
            break;
    }
    numAccounts = i;
    i = 0;
    
    fseek(fp,pos,SEEK_SET);
    while(fgets(line,sizeof(line),fp)){
        sscanf(line, "Transfer %d %d %d", &(records[i].from),&(records[i].to),&(records[i].amount));
        i++;
    }
    numTransfers = i;

    /*initialization*/
    pthread_mutex_init(&lockTransfer,NULL);
    pthread_mutex_init(&lockState,NULL);
    for(i=0;i<numAccounts;i++){ // initial a lock and conditional variables for each account
        pthread_mutex_init(&locklist[i],NULL);
        pthread_cond_init(&condlist[i],NULL);
    }
	
 
    /*set up workers and join*/
    int ret;
    struct arguments arg = {.arg1 = numAccounts,.arg2 = numTransfers};
    pthread_t threads[SIZE];
    for(i=0;i<numWorkers;i++){
	    ret = pthread_create(&threads[i],NULL,worker_routine,(void *)&arg);
	    if(ret){
		    errno = ret;
		    perror("pthread_create(calendar filter)");
		    return -1;
	    }
    }

    for(i=0;i<numWorkers;i++)
        pthread_join(threads[i], NULL);

    /*print the outcome*/
    for(i=0;i<numAccounts;i++)
        printf("%d %d\n",accounts[i].NO,accounts[i].balance);

    return 0;
}

void *worker_routine(void *arg){
    struct arguments *args = arg;
    const int N = args->arg1; // the number of accounts
    const int M = args->arg2; // the number of records needing to be processed
    int index,i,flag1,flag2;
    int sender, receiver, amount;
    while(1){
        /*check if there is any pending records*/
        pthread_mutex_lock(&lockTransfer);
        if(cur==M){// no transfer left
            pthread_mutex_unlock(&lockTransfer);
            pthread_exit(NULL);
        }
        else{
            index = cur;
            cur++;
            pthread_mutex_unlock(&lockTransfer);
        }
        /*preparation*/
        flag1 = -1;
        flag2 = -1;
        sender = records[index].from;
        receiver = records[index].to;
        amount = records[index].amount;
        for(i=0;i<N;i++){
            if(flag1!=-1 && flag2!=-1)
                break;
            if(accounts[i].NO==sender)
                flag1 = i; 
            else if(accounts[i].NO==receiver)
                flag2 = i;
        }
        //printf("%d %d\n",flag1,flag2);
        if(flag1!=-1 && flag2!=-1){
            /*start processing*/ 
            start_transfer(flag1,flag2);
            /*processing*/ 
            accounts[flag1].balance = accounts[flag1].balance - amount;
            accounts[flag2].balance = accounts[flag2].balance + amount; 
            //printf("%d %d\n",accounts[flag1].balance,accounts[flag2].balance);
            /*end processing*/ 
            end_transfer(flag1,flag2);
        }   
    }  
}
void start_transfer(int from,int to){ // the arguments are the index (not account number) of sender and receiver
    int acc1,acc2;
    if(from>to){
        acc1 = to;
        acc2 = from;
    }
    else{
        acc1 = from;
        acc2 = to;
    }
    /*try to acquire the locks*/
    pthread_mutex_lock(&lockState);
    while(state[acc1]==1){
        pthread_cond_wait(&(condlist[acc1]),&lockState);
    }
    state[acc1]==1;
    while(state[acc2]==1){
        pthread_cond_wait(&(condlist[acc2]),&lockState);
    }
    state[acc2]==1;
    //printf("%d %d\n",state[acc1],state[acc2]);
    pthread_mutex_unlock(&lockState);
    /*acquire the accounts' locks*/
    pthread_mutex_lock(&(locklist[acc1]));
    pthread_mutex_lock(&(locklist[acc2]));
}

void end_transfer(int from,int to){
    /*release the accounts' locks*/
    pthread_mutex_unlock(&(locklist[from]));
    pthread_mutex_unlock(&(locklist[to]));
    /*update the state and signal*/
    pthread_mutex_lock(&lockState);
    state[from] = 0;
    state[to] = 0;
    pthread_cond_signal(&condlist[from]);
    pthread_cond_signal(&condlist[to]);
    pthread_mutex_unlock(&lockState);
}















