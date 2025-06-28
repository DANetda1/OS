#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"

int shm_id;
int sem_id;
SharedData* data;
int registrar_id = 0;

void sem_op(int id, int num, int val) {
    struct sembuf op;
    op.sem_num = static_cast<unsigned short>(num);
    op.sem_op = static_cast<short>(val);
    op.sem_flg = 0;
    semop(id, &op, 1);
}

void init() {
    shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        std::cerr << "Ошибка: общая память не найдена. Запустите controller.\n";
        exit(1);
    }

    data = (SharedData*)shmat(shm_id, nullptr, 0);
    if (data == (void*)-1) {
        std::cerr << "Ошибка подключения к общей памяти.\n";
        exit(1);
    }

    sem_id = semget(SEM_KEY, 0, 0666);
    if (sem_id == -1) {
        std::cerr << "Ошибка подключения к семафорам.\n";
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: ./registrar <ID>\n";
        return 1;
    }

    registrar_id = atoi(argv[1]);
    srand(getpid());
    init();

    while (true) {
        sleep(rand() % 2 + 1);
        int spec = rand() % 3;

        sem_op(sem_id, SEM_MUTEX, -1);

        if (data->registered_patients >= data->total_patients) {
            sem_op(sem_id, SEM_MUTEX, 1);
            break;
        }

        int num = data->next_patient_number++;
        data->registered_patients++;
        data->queue[data->rear] = num;
        data->patient_specialties[data->rear] = spec;
        data->rear = (data->rear + 1) % MAX_QUEUE;

        sem_op(sem_id, SEM_MUTEX, 1);

        std::cout << "[Регистратор " << registrar_id << "] Пациент #" << num
                  << " направлен к врачу: " << specialty_names[spec] << "\n";

        sem_op(sem_id, SEM_READY, 1);
    }

    return 0;
}
