#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
    int fifo_rd = open("fifo1", O_RDONLY);
    if (fifo_rd < 0) {
        fprintf(stderr, "Не удалось открыть fifo1 для чтения\n");
        return 1;
    }
    char buffer[5000];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = read(fifo_rd, buffer, sizeof(buffer));
    close(fifo_rd);
    if (bytesRead <= 0) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "Ошибка чтения из fifo1\n");
        return 1;
    }
    char *p = strchr(buffer, '\n');
    if (!p) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "Неверный формат данных\n");
        return 1;
    }
    *p = 0;
    int N1 = atoi(buffer);
    char *rest = p + 1;
    p = strchr(rest, '\n');
    if (!p) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "Неверный формат данных\n");
        return 1;
    }
    *p = 0;
    int N2 = atoi(rest);
    char *data = p + 1;
    ssize_t dataLen = bytesRead - (data - buffer);
    if (N1 < 0 || N2 < 0) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "N1 и N2 должны быть неотрицательными\n");
        return 1;
    }
    if (N2 < N1) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "Значение N2 должно быть больше чем N1\n");
        return 1;
    }
    if (N2 >= dataLen) {
        int err_wr = open("fifo2", O_WRONLY);
        if (err_wr >= 0) close(err_wr);
        fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
        return 1;
    }
    for (int i = 0; i < (N2 - N1 + 1) / 2; i++) {
        char tmp = data[N1 + i];
        data[N1 + i] = data[N2 - i];
        data[N2 - i] = tmp;
    }
    int fifo_wr = open("fifo2", O_WRONLY);
    if (fifo_wr < 0) {
        fprintf(stderr, "Не удалось открыть fifo2 для записи\n");
        return 1;
    }
    if (write(fifo_wr, data, dataLen) != dataLen) {
        close(fifo_wr);
        fprintf(stderr, "Ошибка записи в fifo2\n");
        return 1;
    }
    close(fifo_wr);
    printf("Программа processor8 выполнена успешно\n");
    return 0;
}
