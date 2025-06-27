#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <csignal>

const int SHM_SIZE = sizeof(int);
const key_t SHM_KEY = 123456;

int shmid;
int *shared_memory;
bool running = true;

void cleanup(int signum) {
    std::cout << "Сервер завершился при помощи - флага -1." << std::endl;
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, nullptr);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shared_memory = static_cast<int*>(shmat(shmid, nullptr, 0));
    if (shared_memory == reinterpret_cast<int*>(-1)) {
        perror("shmat");
        return 1;
    }

    *shared_memory = 0;

    std::cout << "Ожидание клиента..." << std::endl;

    int received_count = 0;

    while (running) {
        if (*shared_memory != 0) {
            if (*shared_memory == -1) {
                running = false;
            } else {
                std::cout << "Принято: " << *shared_memory << std::endl;
                received_count++;
            }
            *shared_memory = 0;
        }
        usleep(100000);
    }

    cleanup(0);
    return 0;
}
