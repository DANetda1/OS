#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <csignal>

const int SHM_SIZE = 1024;
const key_t SHM_KEY = 123321;

int shmid;
int *shared_memory;

void cleanup(int signum) {
    std::cout << "Клиент завершился при помощи - Ctrl+C." << std::endl;
    shmdt(shared_memory);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);
    srand(time(nullptr));

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    shared_memory = (int*)shmat(shmid, nullptr, 0);

    while (true) {
        if (*shared_memory == 0) {
            int num = rand() % 100 + 1;
            *shared_memory = num;
            std::cout << "Отправлено: " << num << std::endl;
        }
        usleep(250000);
    }

    return 0;
}
