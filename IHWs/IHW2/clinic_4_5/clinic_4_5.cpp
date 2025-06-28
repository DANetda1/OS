#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>

#define MAX_QUEUE 100

enum Specialty { DENTIST = 0, SURGEON = 1, THERAPIST = 2 };
const char* specialty_names_ru[] = { "Стоматолог", "Хирург", "Терапевт" };

struct SharedData {
    int queue[MAX_QUEUE];
    int patient_numbers[MAX_QUEUE];
    int patient_specialties[MAX_QUEUE];
    int front, rear;
    int next_patient_number;
    int treated_patients;
    int total_patients;
    sem_t queue_mutex;
    sem_t patient_ready;
    sem_t doctor_sem[3];
    sem_t treated_mutex;
};

SharedData* data;
sem_t* sem_print;
pid_t parent_pid;

void cleanup() {
    munmap(data, sizeof(SharedData));
    sem_close(sem_print);
    sem_unlink("/print_mutex");
}

void signal_handler(int sig) {
    if (getpid() == parent_pid)
        std::cout << "\nПрограмма завершена по сигналу.\n";
    cleanup();
    exit(0);
}

void enqueue(int specialty, int number) {
    sem_wait(&data->queue_mutex);
    data->queue[data->rear] = number;
    data->patient_specialties[data->rear] = specialty;
    data->rear = (data->rear + 1) % MAX_QUEUE;
    sem_post(&data->queue_mutex);
    sem_post(&data->patient_ready);
}

bool dequeue(int& number, int& specialty) {
    sem_wait(&data->patient_ready);
    sem_wait(&data->queue_mutex);
    int index = data->front;
    number = data->queue[index];
    specialty = data->patient_specialties[index];
    data->front = (data->front + 1) % MAX_QUEUE;
    sem_post(&data->queue_mutex);
    return true;
}

void registrar(int id, int patients_per_registrar) {
    srand(getpid());
    for (int i = 0; i < patients_per_registrar; ++i) {
        sleep(rand() % 2 + 1);
        int specialty = rand() % 3;
        sem_wait(&data->queue_mutex);
        int number = data->next_patient_number++;
        sem_post(&data->queue_mutex);
        sem_wait(sem_print);
        std::cout << "[Регистратор " << id << "] Пациент #" << number
                  << " направлен к врачу: " << specialty_names_ru[specialty] << std::endl;
        sem_post(sem_print);
        enqueue(specialty, number);
    }
    exit(0);
}

void doctor(Specialty spec) {
    srand(getpid());
    while (true) {
        int number, patient_spec;
        bool got = dequeue(number, patient_spec);
        if (!got || patient_spec != spec) {
            enqueue(patient_spec, number);
            usleep(100000);
            continue;
        }
        sem_wait(&data->doctor_sem[spec]);
        int duration = rand() % 3 + 1;
        sem_wait(sem_print);
        std::cout << "[" << specialty_names_ru[spec]
                  << "] Приём пациента #" << number << " (" << duration << " сек)..." << std::endl;
        sem_post(sem_print);
        sleep(duration);
        sem_wait(sem_print);
        std::cout << "[" << specialty_names_ru[spec]
                  << "] Пациент #" << number << " выписан." << std::endl;
        sem_post(sem_print);
        sem_post(&data->doctor_sem[spec]);
        sem_wait(&data->treated_mutex);
        data->treated_patients++;
        sem_post(&data->treated_mutex);
    }
}

int main(int argc, char* argv[]) {
    parent_pid = getpid();
    signal(SIGINT, signal_handler);

    if (argc != 2) {
        std::cerr << "Использование: ./clinic_4_5 <кол-во_пациентов>\n";
        return 1;
    }

    int total_patients = atoi(argv[1]);
    if (total_patients <= 0 || total_patients > MAX_QUEUE) {
        std::cerr << "Некорректное количество пациентов (1 - " << MAX_QUEUE << ")\n";
        return 1;
    }

    data = (SharedData*)mmap(NULL, sizeof(SharedData),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) return 1;

    data->front = data->rear = 0;
    data->next_patient_number = 1;
    data->treated_patients = 0;
    data->total_patients = total_patients;

    sem_init(&data->queue_mutex, 1, 1);
    sem_init(&data->treated_mutex, 1, 1);
    sem_init(&data->patient_ready, 1, 0);
    for (int i = 0; i < 3; ++i)
        sem_init(&data->doctor_sem[i], 1, 1);

    sem_print = sem_open("/print_mutex", O_CREAT, 0600, 1);

    pid_t pids[10];
    int idx = 0;

    int per_registrar = total_patients / 2;
    int last_extra = total_patients % 2;

    for (int i = 0; i < 2; ++i) {
        int patients = per_registrar + (i == 1 ? last_extra : 0);
        if ((pids[idx++] = fork()) == 0)
            registrar(i + 1, patients);
    }

    for (int i = 0; i < 3; ++i) {
        if ((pids[idx++] = fork()) == 0)
            doctor((Specialty)i);
    }

    for (int i = 0; i < 2; ++i)
        waitpid(pids[i], NULL, 0);

    while (true) {
        sem_wait(&data->treated_mutex);
        int treated = data->treated_patients;
        sem_post(&data->treated_mutex);
        if (treated >= total_patients) break;
        usleep(500000);
    }

    std::cout << "\nКлиника завершает работу. Завершаем процессы врачей...\n";
    for (int i = 2; i < 5; ++i)
        kill(pids[i], SIGTERM);

    sleep(1);
    cleanup();
    return 0;
}
