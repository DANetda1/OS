#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Неверное количество параметров\n");
        fprintf(stderr, "Использование: %s <входной файл> <выходной файл> <N1> <N2>\n", argv[0]);
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
    if (N2 < N1) {
        fprintf(stderr, "Значение N2 должно быть больше чем N1\n");
        return 1;
    }
    int fd_in = open(inputFile, O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Не удалось открыть входной файл\n");
        return 1;
    }
    char buffer[5000];
    memset(buffer, 0, sizeof(buffer));
    int paramLen = snprintf(buffer, sizeof(buffer), "%d\n%d\n", N1, N2);
    if (paramLen <= 0 || paramLen >= (int)sizeof(buffer)) {
        close(fd_in);
        fprintf(stderr, "Ошибка при формировании параметров\n");
        return 1;
    }
    ssize_t spaceForFile = 5000 - paramLen;
    ssize_t bytesRead = read(fd_in, buffer + paramLen, spaceForFile);
    close(fd_in);
    if (bytesRead < 0) {
        fprintf(stderr, "Ошибка чтения из входного файла\n");
        return 1;
    }
    if (bytesRead == 0) {
        int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out >= 0) close(fd_out);
        printf("Программа writer8 выполнена успешно (пустой файл)\n");
        return 0;
    }
    ssize_t totalSize = paramLen + bytesRead;
    mkfifo("fifo1", 0666);
    mkfifo("fifo2", 0666);
    int fifo_wr = open("fifo1", O_WRONLY);
    if (fifo_wr < 0) {
        fprintf(stderr, "Не удалось открыть fifo1 для записи\n");
        return 1;
    }
    if (write(fifo_wr, buffer, totalSize) != totalSize) {
        close(fifo_wr);
        fprintf(stderr, "Ошибка записи в fifo1\n");
        return 1;
    }
    close(fifo_wr);
    memset(buffer, 0, sizeof(buffer));
    int fifo_rd = open("fifo2", O_RDONLY);
    if (fifo_rd < 0) {
        fprintf(stderr, "Не удалось открыть fifo2 для чтения\n");
        return 1;
    }
    ssize_t received = read(fifo_rd, buffer, sizeof(buffer));
    close(fifo_rd);
    if (received < 0) {
        fprintf(stderr, "Ошибка чтения из fifo2\n");
        return 1;
    }
    int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        fprintf(stderr, "Не удалось открыть выходной файл\n");
        return 1;
    }
    if (write(fd_out, buffer, received) != received) {
        close(fd_out);
        fprintf(stderr, "Ошибка записи в выходной файл\n");
        return 1;
    }
    close(fd_out);
    printf("Программа writer8 выполнена успешно\n");
    unlink("fifo1");
    unlink("fifo2");
    return 0;
}
