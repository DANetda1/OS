#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <csignal>
#include <unistd.h>

const int SHM_SIZE = 1024;
const key_t SHM_KEY = 123321;

int shmid;
int *shared_memory;

void cleanup(int signum) {
    std::cout << "Сервер завершился при помощи - Crtl+C." << std::endl;
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, nullptr);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);
    
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    shared_memory = (int*)shmat(shmid, nullptr, 0);
    
    while (true) {
        if (*shared_memory != 0) {
            std::cout << "Принято: " << *shared_memory << std::endl;
            *shared_memory = 0;
        }
        usleep(250000);
    }
    return 0;
}
