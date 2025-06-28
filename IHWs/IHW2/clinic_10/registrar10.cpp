#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include "common.h"

int qd,qs,qt,semid,shmid;
int* counter_ptr;

void bye(int){ _exit(0); }

int main(int c,char* v[]){
    if(c!=3){ std::cerr<<"usage: registrar10 ID CNT\n"; return 1; }
    int id=std::atoi(v[1]),cnt=std::atoi(v[2]);
    qd=msgget(MSG_D,0); qs=msgget(MSG_S,0); qt=msgget(MSG_T,0);
    semid=semget(SEM_KEY,0,0); shmid=shmget(SHM_KEY,0,0);
    counter_ptr=(int*)shmat(shmid,nullptr,0);
    signal(SIGTERM,bye); srand(getpid());
    struct sembuf lock{SEM_COUNTER,-1,0},unlock{SEM_COUNTER,1,0};
    struct sembuf plock{SEM_PRINT,-1,0},punlock{SEM_PRINT,1,0};
    for(int i=0;i<cnt;i++){
        sleep(rand()%2+1);
        int spec=rand()%3;
        semop(semid,&lock,1); int num=(*counter_ptr)++; semop(semid,&unlock,1);
        Msg m{1,num};
        int q=(spec==DENTIST)?qd:(spec==SURGEON)?qs:qt;
        msgsnd(q,&m,sizeof(int),0);
        semop(semid,&plock,1);
        std::cout<<"[Регистратор "<<id<<"] Пациент #"<<num<<" направлен к врачу: "<<spec_ru[spec]<<"\n";
        semop(semid,&punlock,1);
    }
    return 0;
}
