#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include "common.h"

mqd_t  q;
sem_t *sem_treated, *sem_print;

void bye(int)
{
    mq_close(q);
    sem_close(sem_treated);
    sem_close(sem_print);
    _exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    int spec = std::atoi(argv[1]);

    q = spec == DENTIST ? mq_open(QUEUE_D, O_RDONLY) :
        spec == SURGEON ? mq_open(QUEUE_S, O_RDONLY) :
                          mq_open(QUEUE_T, O_RDONLY);

    sem_treated = sem_open(SEM_TREATED, 0);
    sem_print   = sem_open(SEM_PRINT,   0);

    signal(SIGTERM, bye);
    srand(getpid());

    PatientMsg msg;
    while (true)
    {
        if (mq_receive(q, reinterpret_cast<char*>(&msg), sizeof(msg), nullptr) < 0)
            continue;

        if (msg.number == 0) break;

        int dur = rand() % 3 + 1;
        sem_wait(sem_print);
        std::cout << "[" << specialty_names_ru[spec]
                  << "] Приём пациента #" << msg.number
                  << " (" << dur << " сек)...\n";
        sem_post(sem_print);

        sleep(dur);

        sem_wait(sem_print);
        std::cout << "[" << specialty_names_ru[spec]
                  << "] Пациент #" << msg.number << " выписан.\n";
        sem_post(sem_print);

        sem_post(sem_treated);
    }

    bye(0);
    return 0;
}
