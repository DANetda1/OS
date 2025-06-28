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
int spec;

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
    sem_id = semget(SEM_KEY, 0, 0666);
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;
    spec = atoi(argv[1]);
    srand(getpid());
    init();

    while (true) {
        sem_op(sem_id, SEM_READY, -1);
        sem_op(sem_id, SEM_MUTEX, -1);
        int num = -1, pspec = -1;
        if (data->front != data->rear) {
            int idx = data->front;
            if (data->patient_specialties[idx] == spec) {
                num = data->queue[idx];
                data->front = (data->front + 1) % MAX_QUEUE;
                pspec = spec;
            }
        }
        sem_op(sem_id, SEM_MUTEX, 1);

        if (pspec == spec && num != -1) {
            sem_op(sem_id, SEM_DOCTOR_BASE + spec, -1);
            int dur = rand() % 3 + 1;
            std::cout << "[" << specialty_names[spec]
                      << "] Приём пациента #" << num
                      << " (" << dur << " сек)\n";
            sleep(dur);
            std::cout << "[" << specialty_names[spec]
                      << "] Пациент #" << num << " выписан\n";
            sem_op(sem_id, SEM_DOCTOR_BASE + spec, 1);
            sem_op(sem_id, SEM_TREATED, -1);
            data->treated_patients++;
            sem_op(sem_id, SEM_TREATED, 1);
        } else {
            usleep(100000);
            sem_op(sem_id, SEM_READY, 1);
        }

        if (data->treated_patients >= data->total_patients) {
            sem_op(sem_id, SEM_MUTEX, -1);
            if (!data->done_message_shown) {
                data->done_message_shown = 1;
                sem_op(sem_id, SEM_MUTEX, 1);
                std::cout << "\nКлиника завершает работу. Все пациенты вылечены. Нажмите Ctrl+C для выхода.\n";
            } else {
                sem_op(sem_id, SEM_MUTEX, 1);
            }
            break;
        }
    }

    return 0;
}
