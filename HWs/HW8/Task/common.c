#include "common.h"
#include <semaphore.h>

const char* shar_object = "/posix-shar-object";
int buf_id;
shared_memory *buffer;

const char *full_sem_name = "/full-semaphore";
sem_t *full;

const char *empty_sem_name = "/empty-semaphore";
sem_t *empty;

const char *mutex_writer_name = "/writer-mutex";
sem_t *mutex_writer;

const char *mutex_reader_name = "/reader-mutex";
sem_t *mutex_reader;

const char *admin_sem_name = "/admin-semaphore";
sem_t *admin;

void init(void) {
  if ((admin = sem_open(admin_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: admin");
    exit(-1);
  }
  if ((mutex_writer = sem_open(mutex_writer_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: writer mutex");
    exit(-1);
  }
  if ((mutex_reader = sem_open(mutex_reader_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: reader mutex");
    exit(-1);
  }
  if ((empty = sem_open(empty_sem_name, O_CREAT, 0666, BUF_SIZE)) == 0) {
    perror("sem_open: empty");
    exit(-1);
  }
  if ((full = sem_open(full_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: full");
    exit(-1);
  }
}

void close_common_semaphores(void) {
  sem_close(empty);
  sem_close(full);
  sem_close(admin);
  sem_close(mutex_writer);
  sem_close(mutex_reader);
}

void unlink_all(void) {
  sem_unlink(empty_sem_name);
  sem_unlink(full_sem_name);
  sem_unlink(admin_sem_name);
  sem_unlink(mutex_writer_name);
  sem_unlink(mutex_reader_name);
  shm_unlink(shar_object);
}
