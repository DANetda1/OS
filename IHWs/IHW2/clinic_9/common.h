#ifndef COMMON_H
#define COMMON_H

#include <mqueue.h>

#define QUEUE_D "/q_d"
#define QUEUE_S "/q_s"
#define QUEUE_T "/q_t"

#define SEM_TREATED "/treated_sem"
#define SEM_PRINT   "/print_mutex"

#define COUNTER_SHM "/clinic_counter"
#define SEM_COUNTER "/counter_mutex"

struct PatientMsg { int number; };

enum Specialty { DENTIST = 0, SURGEON = 1, THERAPIST = 2 };

static const char* specialty_names_ru[] = { "Стоматолог", "Хирург", "Терапевт" };

#endif
