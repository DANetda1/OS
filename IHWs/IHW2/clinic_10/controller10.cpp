#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "common.h"

int qd, qs, qt, semid, shmid;
int* counter_ptr;
int total;

void sem_set(int n, int v){ semctl(semid,n,SETVAL,v); }

void send_stop(){ Msg m{1,0}; msgsnd(qd,&m,sizeof(int),0); msgsnd(qs,&m,sizeof(int),0); msgsnd(qt,&m,sizeof(int),0); }

void cleanup(int){
    send_stop();
    system("pkill -TERM doctor10 2>/dev/null");
    system("pkill -TERM registrar10 2>/dev/null");
    msgctl(qd,IPC_RMID,nullptr); msgctl(qs,IPC_RMID,nullptr); msgctl(qt,IPC_RMID,nullptr);
    semctl(semid,0,IPC_RMID);
    shmdt(counter_ptr); shmctl(shmid,IPC_RMID,nullptr);
    std::cout<<"\nРесурсы удалены\n";
    _exit(0);
}

int main(int c,char* v[]){
    if(c!=2){ std::cerr<<"usage: controller10 N\n"; return 1; }
    total=std::atoi(v[1]);
    qd=msgget(MSG_D,0666|IPC_CREAT); qs=msgget(MSG_S,0666|IPC_CREAT); qt=msgget(MSG_T,0666|IPC_CREAT);
    semid=semget(SEM_KEY,3,0666|IPC_CREAT);
    sem_set(SEM_TREATED,0); sem_set(SEM_PRINT,1); sem_set(SEM_COUNTER,1);
    shmid=shmget(SHM_KEY,sizeof(int),0666|IPC_CREAT);
    counter_ptr=(int*)shmat(shmid,nullptr,0); *counter_ptr=1;
    signal(SIGINT,cleanup); signal(SIGTERM,cleanup);
    struct sembuf p{SEM_TREATED,-1,0};
    for(int i=0;i<total;i++) semop(semid,&p,1);
    struct sembuf lock{SEM_PRINT,-1,0},unlock{SEM_PRINT,1,0};
    semop(semid,&lock,1);
    std::cout<<"\nКлиника завершает работу. Все пациенты вылечены.\n";
    semop(semid,&unlock,1);
    cleanup(0);
    return 0;
}
