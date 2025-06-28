#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define CHUNK_SIZE 128

void reverse_substring(char *data, int start, int end) {
    while (start < end) {
        char tmp = data[start];
        data[start] = data[end];
        data[end] = tmp;
        start++;
        end--;
    }
}

int main() {
    int fifo_rd = open("fifo1", O_RDONLY);
    if (fifo_rd < 0) {
        fprintf(stderr, "Не удалось открыть fifo1 для чтения\n");
        return 1;
    }
    char *accum = NULL;
    size_t accumSize = 0;
    size_t accumCap = 0;
    while (1) {
        char buf[CHUNK_SIZE];
        ssize_t rd = read(fifo_rd, buf, CHUNK_SIZE);
        if (rd < 0) {
            free(accum);
            close(fifo_rd);
            fprintf(stderr, "Ошибка чтения из fifo1\n");
            return 1;
        }
        if (rd == 0) {
            break;
        }
        if (accumSize + rd > accumCap) {
            size_t newCap = accumCap == 0 ? 1024 : accumCap * 2;
            while (newCap < accumSize + rd) {
                newCap *= 2;
            }
            char *temp = realloc(accum, newCap);
            if (!temp) {
                free(accum);
                close(fifo_rd);
                fprintf(stderr, "Недостаточно памяти\n");
                return 1;
            }
            accum = temp;
            accumCap = newCap;
        }
        memcpy(accum + accumSize, buf, rd);
        accumSize += rd;
    }
    close(fifo_rd);
    if (!accum || accumSize == 0) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "Нет данных для обработки\n");
        return 1;
    }
    int lineCount = 0;
    char *p = memchr(accum, '\n', accumSize);
    if (!p) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "Неверный формат (нет первого переноса строки)\n");
        return 1;
    }
    *p = 0;
    int N1 = atoi(accum);
    lineCount++;
    char *rest = p + 1;
    size_t restLen = accumSize - (rest - accum);
    p = memchr(rest, '\n', restLen);
    if (!p) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "Неверный формат (нет второго переноса строки)\n");
        return 1;
    }
    *p = 0;
    int N2 = atoi(rest);
    lineCount++;
    char *dataStart = p + 1;
    size_t dataLen = accumSize - (dataStart - accum);
    if (N1 < 0 || N2 < 0) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "N1 и N2 должны быть неотрицательными\n");
        return 1;
    }
    if (N2 <= N1) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "Значение N2 должно быть больше N1\n");
        return 1;
    }
    if ((size_t)N2 >= dataLen) {
        int fifo_err = open("fifo2", O_WRONLY);
        if (fifo_err >= 0) close(fifo_err);
        free(accum);
        fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
        return 1;
    }
    reverse_substring(dataStart, N1, N2);
    int fifo_wr = open("fifo2", O_WRONLY);
    if (fifo_wr < 0) {
        free(accum);
        fprintf(stderr, "Не удалось открыть fifo2 для записи\n");
        return 1;
    }
    size_t pos = 0;
    while (pos < dataLen) {
        size_t toWrite = dataLen - pos;
        if (toWrite > CHUNK_SIZE) {
            toWrite = CHUNK_SIZE;
        }
        ssize_t wr = write(fifo_wr, dataStart + pos, toWrite);
        if (wr < 0 || (size_t)wr != toWrite) {
            close(fifo_wr);
            free(accum);
            fprintf(stderr, "Ошибка записи в fifo2\n");
            return 1;
        }
        pos += toWrite;
    }
    close(fifo_wr);
    free(accum);
    printf("Программа processor9 выполнена успешно\n");
    return 0;
}
