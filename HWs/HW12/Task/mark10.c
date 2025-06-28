#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

union semun { int val; struct semid_ds *buf; unsigned short *array; struct seminfo *__buf; };

volatile sig_atomic_t terminate = 0;

void handler(int s) { terminate = 1; }

void sem_do(int id, int op) {
    struct sembuf sb = {0, op, 0};
    while (semop(id, &sb, 1) == -1) {
        if (errno == EINTR && terminate) _exit(0);
        if (errno != EINTR) { perror("semop"); _exit(1); }
    }
}

int main() {
    int fd[2];
    if (pipe(fd)) { perror("pipe"); return 1; }

    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semid == -1) { perror("semget"); return 1; }

    union semun arg; arg.val = 0;
    if (semctl(semid, 0, SETVAL, arg) == -1) { perror("semctl"); return 1; }

    signal(SIGINT, handler);

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        int cnt = 0;
        char buf[256];
        while (!terminate) {
            sem_do(semid, -1);
            ssize_t n = read(fd[0], buf, sizeof(buf));
            if (n <= 0) break;
            printf("Дочерний процесс получил: %s\n", buf);
            snprintf(buf, sizeof(buf), "Ответ %d от дочернего процесса", cnt++);
            write(fd[1], buf, strlen(buf) + 1);
            sleep(1);
            sem_do(semid, 1);
        }
        close(fd[0]);
        close(fd[1]);
        _exit(0);
    } else {
        int cnt = 0;
        char buf[256];
        while (!terminate) {
            snprintf(buf, sizeof(buf), "Сообщение %d от родителя", cnt++);
            write(fd[1], buf, strlen(buf) + 1);
            sleep(1);
            sem_do(semid, 1);
            sem_do(semid, -1);
            ssize_t n = read(fd[0], buf, sizeof(buf));
            if (n <= 0) break;
            printf("Родительский процесс получил: %s\n", buf);
        }
        printf("Получен сигнал завершения с клавиатуры, программа завершает свою работу\n");
        kill(pid, SIGINT);
        wait(NULL);
        close(fd[0]);
        close(fd[1]);
        semctl(semid, 0, IPC_RMID);
    }
    return 0;
}
