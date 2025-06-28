#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Неверное количество параметров\n");
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
            size_t newCap = (capacity == 0 ? 8192 : capacity * 2);
            char *temp = realloc(fileData, newCap);
            if (!temp) {
                free(fileData);
                close(fd_in);
                fprintf(stderr, "Недостаточно памяти\n");
                return 1;
            }
            fileData = temp;
            capacity = newCap;
        }
        ssize_t rd = read(fd_in, fileData + fileSize, capacity - fileSize);
        if (rd < 0) {
            free(fileData);
            close(fd_in);
            fprintf(stderr, "Ошибка чтения входного файла\n");
            return 1;
        }
        if (rd == 0) {
            break;
        }
        fileSize += rd;
    }
    close(fd_in);

    if (fileSize > 0 && (size_t)N2 >= fileSize) {
        free(fileData);
        fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
        return 1;
    }

    if (mkfifo("fifo1", 0666) == -1 && errno != EEXIST) {
        free(fileData);
        fprintf(stderr, "Не удалось создать fifo1\n");
        return 1;
    }
    if (mkfifo("fifo2", 0666) == -1 && errno != EEXIST) {
        free(fileData);
        unlink("fifo1");
        fprintf(stderr, "Не удалось создать fifo2\n");
        return 1;
    }

    int fifo_wr = open("fifo1", O_WRONLY);
    if (fifo_wr < 0) {
        free(fileData);
        unlink("fifo1");
        unlink("fifo2");
        fprintf(stderr, "Не удалось открыть fifo1 для записи\n");
        return 1;
    }
    char paramBuf[64];
    int plen = snprintf(paramBuf, sizeof(paramBuf), "%d\n%d\n", N1, N2);
    if (plen <= 0 || plen >= (int)sizeof(paramBuf)) {
        free(fileData);
        close(fifo_wr);
        unlink("fifo1");
        unlink("fifo2");
        fprintf(stderr, "Ошибка формирования параметров\n");
        return 1;
    }
    if (write(fifo_wr, paramBuf, plen) != plen) {
        free(fileData);
        close(fifo_wr);
        unlink("fifo1");
        unlink("fifo2");
        fprintf(stderr, "Ошибка записи параметров в fifo1\n");
        return 1;
    }

    size_t sent = 0;
    while (sent < fileSize) {
        size_t toWrite = fileSize - sent;
        if (toWrite > BUF_SIZE) {
            toWrite = BUF_SIZE;
        }
        ssize_t wr = write(fifo_wr, fileData + sent, toWrite);
        if (wr < 0 || (size_t)wr != toWrite) {
            free(fileData);
            close(fifo_wr);
            unlink("fifo1");
            unlink("fifo2");
            fprintf(stderr, "Ошибка записи данных в fifo1\n");
            return 1;
        }
        sent += toWrite;
    }
    free(fileData);
    close(fifo_wr);

    int fifo_rd = open("fifo2", O_RDONLY);
    if (fifo_rd < 0) {
        fprintf(stderr, "Не удалось открыть fifo2 для чтения\n");
        unlink("fifo1");
        unlink("fifo2");
        return 1;
    }
    int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        close(fifo_rd);
        unlink("fifo1");
        unlink("fifo2");
        fprintf(stderr, "Не удалось открыть выходной файл\n");
        return 1;
    }
    while (1) {
        char buf[BUF_SIZE];
        ssize_t rd = read(fifo_rd, buf, BUF_SIZE);
        if (rd < 0) {
            close(fd_out);
            close(fifo_rd);
            unlink("fifo1");
            unlink("fifo2");
            fprintf(stderr, "Ошибка чтения из fifo2\n");
            return 1;
        }
        if (rd == 0) {
            break;
        }
        if (write(fd_out, buf, rd) != rd) {
            close(fd_out);
            close(fifo_rd);
            unlink("fifo1");
            unlink("fifo2");
            fprintf(stderr, "Ошибка записи в выходной файл\n");
            return 1;
        }
    }
    close(fd_out);
    close(fifo_rd);
    unlink("fifo1");
    unlink("fifo2");
    printf("Программа writer9 выполнена успешно\n");
    return 0;
}
