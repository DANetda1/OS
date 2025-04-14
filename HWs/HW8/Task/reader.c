#include "common.h"
#include <unistd.h>

const char *reader_sem_name = "/reader-semaphore";
sem_t *reader;

void sigfunc(int sig) {
  if(sig != SIGINT && sig != SIGTERM) return;
  if(sig == SIGINT) {
    kill(buffer->writer_pid, SIGTERM);
    printf("Reader(SIGINT) ---> Writer(SIGTERM)\n");
  } else if(sig == SIGTERM) {
    printf("Reader(SIGTERM) <--- Writer(SIGINT)\n");
  }
  sem_close(reader);
  printf("Reader: bye!!!\n");
  exit(10);
}

int factorial(int n) {
  int p = 1;
  for(int i = 1; i <= n; ++i) p *= i;
  return p;
}

int main() {
  signal(SIGINT, sigfunc);
  signal(SIGTERM, sigfunc);

  srand(time(0));
  init();

  sem_wait(admin);
  printf("Consumer %d started\n", getpid());
  sem_post(admin);

  if ((buf_id = shm_open(shar_object, O_RDWR, 0666)) == -1) {
    perror("shm_open");
    exit(-1);
  }
  ftruncate(buf_id, sizeof(shared_memory));
  buffer = mmap(0, sizeof(shared_memory), PROT_WRITE|PROT_READ, MAP_SHARED, buf_id, 0);
  if (buffer == (shared_memory*)-1) {
    perror("reader: mmap");
    exit(-1);
  }

  if((reader = sem_open(reader_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: reader");
    exit(-1);
  }
  sem_wait(reader);
  if(buffer->have_reader >= 2) {
    printf("Reader %d: I have lost this work :(\n", getpid());
    sem_post(reader);
    exit(13);
  }
  buffer->have_reader += 1;
  sem_post(reader);
  buffer->reader_pid = getpid();

  while (1) {
    sleep(rand() % 3 + 1);
    sem_wait(full);
    sem_wait(mutex_reader);

    int index = buffer->tail;
    int value = buffer->store[index];
    buffer->store[index] = -1;
    buffer->tail = (buffer->tail + 1) % BUF_SIZE;

    int f = factorial(value);
    printf("Consumer %d: Reads value = %d from cell [%d], factorial = %d\n", getpid(), value, index, f);

    sem_post(empty);
    sem_post(mutex_reader);
  }

  return 0;
}
