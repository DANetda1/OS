#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include "common.h"

sem_t *sem_treated, *sem_print, *sem_counter;
mqd_t qd, qs, qt;
int  total;
int *counter_ptr;

void send_stop()
{
    PatientMsg stop{0};
    mq_send(qd, reinterpret_cast<char*>(&stop), sizeof(stop), 0);
    mq_send(qs, reinterpret_cast<char*>(&stop), sizeof(stop), 0);
    mq_send(qt, reinterpret_cast<char*>(&stop), sizeof(stop), 0);
}

void cleanup(int)
{
    send_stop();
    system("pkill -TERM doctor9 2>/dev/null");
    system("pkill -TERM registrar9 2>/dev/null");

    mq_close(qd); mq_close(qs); mq_close(qt);
    mq_unlink(QUEUE_D); mq_unlink(QUEUE_S); mq_unlink(QUEUE_T);

    sem_close(sem_treated); sem_close(sem_print); sem_close(sem_counter);
    sem_unlink(SEM_TREATED); sem_unlink(SEM_PRINT); sem_unlink(SEM_COUNTER);

    munmap(counter_ptr, sizeof(int));
    shm_unlink(COUNTER_SHM);

    std::cout << "\nРесурсы удалены\n";
    _exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Использование: ./controller9 <кол-во_пациентов>\n";
        return 1;
    }
    total = std::atoi(argv[1]);

    struct mq_attr attr{}; attr.mq_maxmsg = 10; attr.mq_msgsize = sizeof(PatientMsg);
    qd = mq_open(QUEUE_D, O_CREAT | O_RDONLY, 0666, &attr);
    qs = mq_open(QUEUE_S, O_CREAT | O_RDONLY, 0666, &attr);
    qt = mq_open(QUEUE_T, O_CREAT | O_RDONLY, 0666, &attr);

    sem_treated = sem_open(SEM_TREATED, O_CREAT, 0666, 0);
    sem_print   = sem_open(SEM_PRINT,   O_CREAT, 0666, 1);
    sem_counter = sem_open(SEM_COUNTER, O_CREAT, 0666, 1);

    int fd = shm_open(COUNTER_SHM, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(int));
    counter_ptr = static_cast<int*>(mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    *counter_ptr = 1;
    close(fd);

    signal(SIGINT,  cleanup);
    signal(SIGTERM, cleanup);

    int treated = 0;
    while (treated < total)
    {
        sem_wait(sem_treated);
        ++treated;
    }

    sem_wait(sem_print);
    std::cout << "\nКлиника завершает работу. Все пациенты вылечены.\n";
    sem_post(sem_print);

    cleanup(0);
    return 0;
}
