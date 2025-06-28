#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define MAX_QUEUE 100
#define SHM_KEY 1234
#define SEM_KEY 5678

#define SEM_MUTEX 0
#define SEM_READY 1
#define SEM_TREATED 2
#define SEM_DOCTOR_BASE 3

enum Specialty { DENTIST = 0, SURGEON = 1, THERAPIST = 2 };
const char* specialty_names[] = { "Стоматолог", "Хирург", "Терапевт" };

struct SharedData {
    int queue[MAX_QUEUE];
    int patient_specialties[MAX_QUEUE];
    int front, rear;
    int next_patient_number;
    int treated_patients;
    int total_patients;
    int registered_patients;
    int done_message_shown;
};

#endif
