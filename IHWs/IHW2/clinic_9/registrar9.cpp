#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "common.h"

int main(int argc, char* argv[]) {
    if (argc != 3) { std::cerr << "Использование: ./registrar9 <ID> <кол-во_пациентов>\n"; return 1; }
    int id  = std::atoi(argv[1]);
    int cnt = std::atoi(argv[2]);
    srand(getpid());
    mqd_t qd = mq_open(QUEUE_D, O_WRONLY);
    mqd_t qs = mq_open(QUEUE_S, O_WRONLY);
    mqd_t qt = mq_open(QUEUE_T, O_WRONLY);
    sem_t* sem_print   = sem_open(SEM_PRINT,   0);
    sem_t* sem_counter = sem_open(SEM_COUNTER, 0);
    int fd = shm_open(COUNTER_SHM, O_RDWR, 0);
    int* counter_ptr = static_cast<int*>(mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);
    for (int i = 0; i < cnt; ++i) {
        sleep(rand() % 2 + 1);
        int spec = rand() % 3;
        sem_wait(sem_counter);
        int num = (*counter_ptr)++;
        sem_post(sem_counter);
        PatientMsg msg{num};
        mqd_t q = spec == DENTIST ? qd : spec == SURGEON ? qs : qt;
        mq_send(q, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
        sem_wait(sem_print);
        std::cout << "[Регистратор " << id << "] Пациент #" << num
                  << " направлен к врачу: " << specialty_names_ru[spec] << std::endl;
        sem_post(sem_print);
    }
    munmap(counter_ptr, sizeof(int));
    return 0;
}
