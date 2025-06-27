#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <ctime>

const int SHM_SIZE = sizeof(int);
const key_t SHM_KEY = 123456;

int shmid;
int *shared_memory;

void cleanup(int signum) {
    std::cout << "Клиент завершился при помощи - флага -1." << std::endl;
    shmdt(shared_memory);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    srand(static_cast<unsigned int>(time(nullptr)));

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shared_memory = static_cast<int*>(shmat(shmid, nullptr, 0));
    if (shared_memory == reinterpret_cast<int*>(-1)) {
        perror("shmat");
        return 1;
    }

    *shared_memory = 42;
    std::cout << "Клиент подключается..." << std::endl;

    while (*shared_memory == 42) {
        usleep(100000);
    }
    std::cout << "Соединение установлено, начинаем отправку данных." << std::endl;

    for (int i = 0; i < 15; i++) {
        while (*shared_memory != 0) {
            usleep(100000);
        }
        int num = rand() % 100 + 1;
        *shared_memory = num;
        std::cout << "Отправлено: " << num << std::endl;
        usleep(250000);
    }

    while (*shared_memory != 0) {
        usleep(100000);
    }
    *shared_memory = -1;
    std::cout << "Отправлено: -1 (сигнал завершения)" << std::endl;

    cleanup(0);
    return 0;
}
