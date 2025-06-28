#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include "common.h"

int q,semid;

void bye(int){ _exit(0); }

int main(int c,char* v[]){
    if(c!=2) return 1;
    int spec=std::atoi(v[1]);
    q=(spec==DENTIST)?msgget(MSG_D,0):(spec==SURGEON)?msgget(MSG_S,0):msgget(MSG_T,0);
    semid=semget(SEM_KEY,0,0);
    signal(SIGTERM,bye); srand(getpid());
    Msg m;
    struct sembuf plock{SEM_PRINT,-1,0},punlock{SEM_PRINT,1,0},treat{SEM_TREATED,1,0};
    while(true){
        msgrcv(q,&m,sizeof(int),0,0);
        if(m.num==0) break;
        int d=rand()%3+1;
        semop(semid,&plock,1);
        std::cout<<"["<<spec_ru[spec]<<"] Приём пациента #"<<m.num<<" ("<<d<<" сек)...\n";
        semop(semid,&punlock,1);
        sleep(d);
        semop(semid,&plock,1);
        std::cout<<"["<<spec_ru[spec]<<"] Пациент #"<<m.num<<" выписан.\n";
        semop(semid,&punlock,1);
        semop(semid,&treat,1);
    }
    bye(0);
    return 0;
}
