#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <ctime>

const int SHM_SIZE = 1024;
const key_t SHM_KEY = 12345;

int shmid;
int *shared_memory;
bool serverActive = true;

void cleanup(int signum) {
    std::cout << "Клиент завершился при помощи - таймера на 10 секунд." << std::endl;
    shmdt(shared_memory);
    exit(0);
}

void checkServerStatus(int signum) {
    if (!serverActive) {
        cleanup(signum);
    } else {
        serverActive = false;
        alarm(1);
    }
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGALRM, checkServerStatus);

    srand(time(nullptr));
    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    shared_memory = (int*)shmat(shmid, nullptr, 0);

    *shared_memory = 42;
    std::cout << "Клиент подключается..." << std::endl;

    while (*shared_memory == 42) {
        usleep(100000);
    }
    std::cout << "Соединение установлено, начинаем отправку данных." << std::endl;

    alarm(1);

    while (true) {
        while (*shared_memory != 0) {
            usleep(100000);
        }
        int num = rand() % 100 + 1;
        *shared_memory = num;
        std::cout << "Отправлено: " << num << std::endl;
        usleep(250000);
        serverActive = true;
    }

    cleanup(0);
    return 0;
}
