#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <csignal>

const int SHM_SIZE = 1024;
const key_t SHM_KEY = 12345;

int shmid;
int *shared_memory;
bool timerStarted = false;

void cleanup(int signum) {
    std::cout << "Сервер завершился при помощи - таймера на 10 секунд." << std::endl;
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, nullptr);
    exit(0);
}

void timerHandler(int signum) {
    cleanup(signum);
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGALRM, timerHandler);

    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    shared_memory = (int*)shmat(shmid, nullptr, 0);
    *shared_memory = 0;

    std::cout << "Ожидание клиента..." << std::endl;

    while (!timerStarted) {
        if (*shared_memory == 42) {
            std::cout << "Клиент подключился. Запускаем таймер!" << std::endl;
            timerStarted = true;
            alarm(10);
            *shared_memory = 0;
        }
        usleep(100000);
    }

    while (true) {
        if (*shared_memory != 0) {
            std::cout << "Принято: " << *shared_memory << std::endl;
            *shared_memory = 0;
        }
        usleep(250000);
    }

    return 0;
}

