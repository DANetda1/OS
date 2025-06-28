#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 128
#define MSGQ_KEY1 0x1234
#define MSGQ_KEY2 0x2345

struct msgbuf {
    long mtype;
    char mtext[BUF_SIZE];
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Неверное количество параметров\n");
	fprintf(stderr, "Вводите: %s <входной файл> <выходной файл> <N1> <N2>\n", argv[0]);
        return 1;
    }
    char *inputFile = argv[1];
    char *outputFile = argv[2];
    int N1 = atoi(argv[3]);
    int N2 = atoi(argv[4]);
    if (N1 < 0 || N2 < 0) {
        fprintf(stderr, "N1 и N2 должны быть неотрицательными\n");
        return 1;
    }
    if (N2 <= N1) {
        fprintf(stderr, "Значение N2 должно быть больше N1\n");
        return 1;
    }
    int fd_in = open(inputFile, O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Не удалось открыть входной файл\n");
        return 1;
    }
    char *fileData = NULL;
    size_t fileSize = 0;
    size_t capacity = 0;
    while (1) {
        if (fileSize == capacity) {
            size_t nc = capacity == 0 ? 8192 : capacity * 2;
            char *t = realloc(fileData, nc);
            if (!t) {
                free(fileData);
                close(fd_in);
                fprintf(stderr, "Недостаточно памяти\n");
                return 1;
            }
            fileData = t;
            capacity = nc;
        }
        ssize_t rd = read(fd_in, fileData + fileSize, capacity - fileSize);
        if (rd < 0) {
            free(fileData);
            close(fd_in);
            fprintf(stderr, "Ошибка чтения входного файла\n");
            return 1;
        }
        if (rd == 0) break;
        fileSize += rd;
    }
    close(fd_in);
    if (fileSize > 0 && (size_t)N2 >= fileSize) {
        free(fileData);
        fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
        return 1;
    }
    int msqid1 = msgget(MSGQ_KEY1, IPC_CREAT | 0666);
    if (msqid1 < 0) {
        free(fileData);
        fprintf(stderr, "Не удалось создать/подключиться к очереди 1\n");
        return 1;
    }
    int msqid2 = msgget(MSGQ_KEY2, IPC_CREAT | 0666);
    if (msqid2 < 0) {
        free(fileData);
        fprintf(stderr, "Не удалось создать/подключиться к очереди 2\n");
        return 1;
    }
    struct msgbuf msg;
    memset(&msg, 0, sizeof(msg));
    msg.mtype = 1;
    int plen = snprintf(msg.mtext, BUF_SIZE, "%d\n%d\n", N1, N2);
    if (plen <= 0 || plen >= BUF_SIZE) {
        free(fileData);
        fprintf(stderr, "Ошибка формирования параметров\n");
        return 1;
    }
    if (msgsnd(msqid1, &msg, plen, 0) < 0) {
        free(fileData);
        fprintf(stderr, "Ошибка отправки параметров\n");
        return 1;
    }
    size_t sent = 0;
    while (sent < fileSize) {
        size_t left = fileSize - sent;
        if (left > (BUF_SIZE - 1)) left = BUF_SIZE - 1;
        memset(&msg, 0, sizeof(msg));
        msg.mtype = 1;
        memcpy(msg.mtext, fileData + sent, left);
        if (msgsnd(msqid1, &msg, left, 0) < 0) {
            free(fileData);
            fprintf(stderr, "Ошибка отправки данных\n");
            return 1;
        }
        sent += left;
    }
    free(fileData);
    memset(&msg, 0, sizeof(msg));
    msg.mtype = 1;
    if (msgsnd(msqid1, &msg, 0, 0) < 0) {
        fprintf(stderr, "Ошибка отправки конца данных\n");
        return 1;
    }
    int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        fprintf(stderr, "Не удалось открыть выходной файл\n");
        return 1;
    }
    while (1) {
        ssize_t rcv = msgrcv(msqid2, &msg, BUF_SIZE, 2, 0);
        if (rcv < 0) {
            close(fd_out);
            fprintf(stderr, "Ошибка чтения результата\n");
            return 1;
        }
        if (rcv == 0) break;
        if (write(fd_out, msg.mtext, rcv) != rcv) {
            close(fd_out);
            fprintf(stderr, "Ошибка записи в выходной файл\n");
            return 1;
        }
    }
    close(fd_out);
    msgctl(msqid1, IPC_RMID, NULL);
    msgctl(msqid2, IPC_RMID, NULL);
    printf("Программа writer10 выполнена успешно\n");
    return 0;
}
