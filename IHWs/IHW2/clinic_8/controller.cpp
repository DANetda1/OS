#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include "common.h"

int shm_id = -1;
int sem_id = -1;
SharedData* data = nullptr;

void cleanup(int) {
    if (data != nullptr) shmdt(data);
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, nullptr);
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    std::cerr << "\nКонтроллер: очищены ресурсы.\n";
    exit(0);
}

void sem_set(int sem_id, int sem_num, int val) {
    semctl(sem_id, sem_num, SETVAL, val);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, cleanup);

    if (argc != 2) {
        std::cerr << "Использование: ./controller <количество_пациентов>\n";
        return 1;
    }

    int total_patients = atoi(argv[1]);
    if (total_patients <= 0) {
        std::cerr << "Ошибка: количество пациентов должно быть положительным числом\n";
        return 1;
    }

    shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        std::cerr << "Ошибка при создании shared memory\n";
        return 1;
    }

    data = (SharedData*)shmat(shm_id, nullptr, 0);
    if (data == (void*)-1) {
        std::cerr << "Ошибка при подключении к shared memory\n";
        return 1;
    }

    memset(data, 0, sizeof(SharedData));
    data->next_patient_number = 1;
    data->total_patients = total_patients;
    data->registered_patients = 0;
    data->done_message_shown = 0;

    sem_id = semget(SEM_KEY, SEM_DOCTOR_BASE + 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        std::cerr << "Ошибка при создании семафоров\n";
        return 1;
    }

    sem_set(sem_id, SEM_MUTEX, 1);
    sem_set(sem_id, SEM_READY, 0);
    sem_set(sem_id, SEM_TREATED, 1);
    for (int i = 0; i < 3; ++i)
        sem_set(sem_id, SEM_DOCTOR_BASE + i, 1);

    std::cout << "Контроллер запущен. Ожидание Ctrl+C для завершения.\n";

    while (true) pause();
}
