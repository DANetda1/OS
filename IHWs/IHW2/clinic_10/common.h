#ifndef COMMON_H
#define COMMON_H

#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define MSG_D 0x111
#define MSG_S 0x112
#define MSG_T 0x113

#define SEM_KEY 0x120
#define SHM_KEY 0x121

#define SEM_TREATED 0
#define SEM_PRINT   1
#define SEM_COUNTER 2

struct Msg { long mtype; int num; };

enum Spec { DENTIST = 0, SURGEON = 1, THERAPIST = 2 };

static const char* spec_ru[] = { "Стоматолог", "Хирург", "Терапевт" };

#endif
