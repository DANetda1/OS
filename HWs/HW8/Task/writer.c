#include "common.h"
#include <semaphore.h>
#include <signal.h>

const char *writer_sem_name = "/writer-semaphore";
sem_t *writer;
const char *first_writer_sem_name = "/first_writer-semaphore";
sem_t *first_writer;

void sigfunc(int sig) {
  if(sig != SIGINT && sig != SIGTERM) return;
  if(sig == SIGINT) {
    kill(buffer->reader_pid, SIGTERM);
    printf("Writer(SIGINT) ---> Reader(SIGTERM)\n");
  } else if(sig == SIGTERM) {
    printf("Writer(SIGTERM) <--- Reader(SIGINT)\n");
  }
  sem_close(writer);
  sem_close(first_writer);
  close_common_semaphores();
  sem_unlink(writer_sem_name);
  sem_unlink(first_writer_sem_name);
  unlink_all();
  printf("Writer: bye!!!\n");
  exit(10);
}

int main() {
  signal(SIGINT, sigfunc);
  signal(SIGTERM, sigfunc);

  srand(time(0));
  init();

  if ((buf_id = shm_open(shar_object, O_CREAT|O_RDWR, 0666)) == -1) {
    perror("shm_open");
    exit(-1);
  }
  ftruncate(buf_id, sizeof(shared_memory));
  buffer = mmap(0, sizeof(shared_memory), PROT_WRITE|PROT_READ, MAP_SHARED, buf_id, 0);
  if (buffer == (shared_memory*)-1) {
    perror("writer: mmap");
    exit(-1);
  }

  if((writer = sem_open(writer_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: writer");
    exit(-1);
  }
  if((first_writer = sem_open(first_writer_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: first_writer");
    exit(-1);
  }

  sem_wait(writer);
  int writer_number = 0;
  sem_getvalue(first_writer, &writer_number);
  if(writer_number == 0) {
    printf("Writer %d: I have lost this work :(\n", getpid());
    sem_post(writer);
    exit(13);
  }
  sem_wait(first_writer);
  sem_post(writer);

  buffer->writer_pid = getpid();
  buffer->head = 0;
  buffer->tail = 0;
  for (int i = 0; i < BUF_SIZE; ++i) buffer->store[i] = -1;

  int is_writers = 0;
  sem_getvalue(admin, &is_writers);
  if (is_writers == 0) sem_post(admin);

  while (1) {
    sem_wait(empty);
    sem_wait(mutex_writer);

    int index = buffer->head;
    buffer->store[index] = rand() % 11;
    buffer->head = (buffer->head + 1) % BUF_SIZE;

    printf("Producer %d writes value = %d to cell [%d]\n", getpid(), buffer->store[index], index);

    sem_post(full);
    sem_post(mutex_writer);

    sleep(rand() % 3 + 1);
  }

  return 0;
}
