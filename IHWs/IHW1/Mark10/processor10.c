#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>

#define BUF_SIZE 128
#define MSGQ_KEY1 0x1234
#define MSGQ_KEY2 0x2345

struct msgbuf {
    long mtype;
    char mtext[BUF_SIZE];
};

static void reverse_substring(char *d, int s, int e) {
    while (s < e) {
        char t = d[s];
        d[s] = d[e];
        d[e] = t;
        s++;
        e--;
    }
}

int main() {
    int msqid1 = msgget(MSGQ_KEY1, IPC_CREAT | 0666);
    if (msqid1 < 0) {
        fprintf(stderr, "Не удалось открыть очередь 1\n");
        return 1;
    }
    int msqid2 = msgget(MSGQ_KEY2, IPC_CREAT | 0666);
    if (msqid2 < 0) {
        fprintf(stderr, "Не удалось открыть очередь 2\n");
        return 1;
    }
    char *accum = NULL;
    size_t accumSize = 0;
    size_t accumCap = 0;
    int N1 = -1, N2 = -1;
    struct msgbuf msg;
    while (1) {
        ssize_t rcv = msgrcv(msqid1, &msg, BUF_SIZE, 1, 0);
        if (rcv < 0) {
            free(accum);
            fprintf(stderr, "Ошибка чтения из очереди\n");
            return 1;
        }
        if (accumSize == 0 && N1 < 0 && N2 < 0) {
            char *nl = memchr(msg.mtext, '\n', rcv);
            if (!nl) {
                free(accum);
                fprintf(stderr, "Неверный формат (нет первого переноса)\n");
                return 1;
            }
            *nl = 0;
            N1 = atoi(msg.mtext);
            char *rest = nl + 1;
            int rlen = rcv - (rest - msg.mtext);
            nl = memchr(rest, '\n', rlen);
            if (!nl) {
                free(accum);
                fprintf(stderr, "Неверный формат (нет второго переноса)\n");
                return 1;
            }
            *nl = 0;
            N2 = atoi(rest);
            if (N1 < 0 || N2 <= N1) {
                free(accum);
                fprintf(stderr, "Параметры N1/N2 некорректны\n");
                return 1;
            }
            char *dataStart = nl + 1;
            int dataLen = rcv - (dataStart - msg.mtext);
            if (dataLen > 0) {
                while (accumSize + dataLen > accumCap) {
                    size_t nc = accumCap == 0 ? 8192 : accumCap * 2;
                    char *temp = realloc(accum, nc);
                    if (!temp) {
                        free(accum);
                        fprintf(stderr, "Недостаточно памяти\n");
                        return 1;
                    }
                    accum = temp;
                    accumCap = nc;
                }
                memcpy(accum + accumSize, dataStart, dataLen);
                accumSize += dataLen;
            }
        } else if (rcv == 0) {
            break;
        } else {
            while (accumSize + rcv > accumCap) {
                size_t nc = accumCap == 0 ? 8192 : accumCap * 2;
                char *temp = realloc(accum, nc);
                if (!temp) {
                    free(accum);
                    fprintf(stderr, "Недостаточно памяти\n");
                    return 1;
                }
                accum = temp;
                accumCap = nc;
            }
            memcpy(accum + accumSize, msg.mtext, rcv);
            accumSize += rcv;
        }
    }
    if (!accum || accumSize == 0) {
        struct msgbuf x;
        memset(&x, 0, sizeof(x));
        x.mtype = 2;
        msgsnd(msqid2, &x, 0, 0);
        free(accum);
        fprintf(stderr, "Нет данных\n");
        return 1;
    }
    if ((size_t)N2 >= accumSize) {
        struct msgbuf x;
        memset(&x, 0, sizeof(x));
        x.mtype = 2;
        msgsnd(msqid2, &x, 0, 0);
        free(accum);
        fprintf(stderr, "N1/N2 выходят за пределы данных\n");
        return 1;
    }
    reverse_substring(accum, N1, N2);
    size_t pos = 0;
    while (pos < accumSize) {
        size_t left = accumSize - pos;
        if (left > (BUF_SIZE - 1)) left = BUF_SIZE - 1;
        memset(&msg, 0, sizeof(msg));
        msg.mtype = 2;
        memcpy(msg.mtext, accum + pos, left);
        if (msgsnd(msqid2, &msg, left, 0) < 0) {
            free(accum);
            fprintf(stderr, "Ошибка отправки результата\n");
            return 1;
        }
        pos += left;
    }
    memset(&msg, 0, sizeof(msg));
    msg.mtype = 2;
    msgsnd(msqid2, &msg, 0, 0);
    free(accum);
    printf("Программа processor10 выполнена успешно\n");
    return 0;
}
