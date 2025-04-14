#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#define BUF_SIZE 10

typedef struct {
  int store[BUF_SIZE];
  int head;
  int tail;
  int have_reader;
  int reader_pid;
  int writer_pid;
} shared_memory;

extern const char* shar_object;
extern int buf_id;
extern shared_memory *buffer;

extern const char *full_sem_name;
extern sem_t *full;

extern const char *empty_sem_name;
extern sem_t *empty;

extern const char *mutex_writer_name;
extern sem_t *mutex_writer;

extern const char *mutex_reader_name;
extern sem_t *mutex_reader;

extern const char *admin_sem_name;
extern sem_t *admin;

void init(void);
void close_common_semaphores(void);
void unlink_all(void);
