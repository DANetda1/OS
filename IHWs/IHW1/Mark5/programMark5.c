#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 5000
#define FIFO1 "/tmp/fifo1"
#define FIFO2 "/tmp/fifo2"

void reverse_substring(char *buf, int start, int end) {
    while (start < end) {
        char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        start++;
        end--;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Неверное количество параметров\n");
        return 1;
    }
    char *inputFile = argv[1];
    char *outputFile = argv[2];
    int N1 = atoi(argv[3]);
    int N2 = atoi(argv[4]);
    int fd_check = open(inputFile, O_RDONLY);
    if (fd_check < 0) {
        fprintf(stderr, "Не удалось открыть входной файл\n");
        return 1;
    }
    close(fd_check);
    if (N2 <= N1) {
        fprintf(stderr, "Значение N2 должно быть больше N1\n");
        return 1;
    }
    if ((mkfifo(FIFO1, 0666) == -1) && (errno != EEXIST)) {
        fprintf(stderr, "Не удалось создать fifo1\n");
        return 1;
    }
    if ((mkfifo(FIFO2, 0666) == -1) && (errno != EEXIST)) {
        fprintf(stderr, "Не удалось создать fifo2\n");
        unlink(FIFO1);
        return 1;
    }
    pid_t pid1 = fork();
    if (pid1 < 0) {
        fprintf(stderr, "Ошибка fork\n");
        unlink(FIFO1);
        unlink(FIFO2);
        return 1;
    }
    if (pid1 == 0) {
        int fifo1_fd = open(FIFO1, O_WRONLY);
        if (fifo1_fd < 0) {
            fprintf(stderr, "Не удалось открыть fifo1 на запись\n");
            exit(1);
        }
        int fd_in = open(inputFile, O_RDONLY);
        if (fd_in < 0) {
            fprintf(stderr, "Не удалось открыть входной файл\n");
            close(fifo1_fd);
            exit(1);
        }
        char buffer[BUF_SIZE];
        ssize_t bytesRead = read(fd_in, buffer, BUF_SIZE);
        close(fd_in);
        if (bytesRead < 0) {
            fprintf(stderr, "Ошибка чтения входного файла\n");
            close(fifo1_fd);
            exit(1);
        }
        if (write(fifo1_fd, buffer, bytesRead) != bytesRead) {
            fprintf(stderr, "Ошибка записи в fifo1\n");
            close(fifo1_fd);
            exit(1);
        }
        close(fifo1_fd);
        exit(0);
    }
    pid_t pid2 = fork();
    if (pid2 < 0) {
        fprintf(stderr, "Ошибка fork\n");
        unlink(FIFO1);
        unlink(FIFO2);
        return 1;
    }
    if (pid2 == 0) {
        int fifo1_fd = open(FIFO1, O_RDONLY);
        if (fifo1_fd < 0) {
            fprintf(stderr, "Не удалось открыть fifo1 на чтение\n");
            exit(1);
        }
        int fifo2_fd = open(FIFO2, O_WRONLY);
        if (fifo2_fd < 0) {
            fprintf(stderr, "Не удалось открыть fifo2 на запись\n");
            close(fifo1_fd);
            exit(1);
        }
        char buffer[BUF_SIZE];
        ssize_t bytesRead = read(fifo1_fd, buffer, BUF_SIZE);
        close(fifo1_fd);
        if (bytesRead < 0) {
            fprintf(stderr, "Ошибка чтения из fifo1\n");
            close(fifo2_fd);
            exit(1);
        }
        if (N2 >= bytesRead) {
            fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
            close(fifo2_fd);
            exit(1);
        }
        reverse_substring(buffer, N1, N2);
        if (write(fifo2_fd, buffer, bytesRead) != bytesRead) {
            fprintf(stderr, "Ошибка записи в fifo2\n");
            close(fifo2_fd);
            exit(1);
        }
        close(fifo2_fd);
        exit(0);
    }
    pid_t pid3 = fork();
    if (pid3 < 0) {
        fprintf(stderr, "Ошибка fork\n");
        unlink(FIFO1);
        unlink(FIFO2);
        return 1;
    }
    if (pid3 == 0) {
        int fifo2_fd = open(FIFO2, O_RDONLY);
        if (fifo2_fd < 0) {
            fprintf(stderr, "Не удалось открыть fifo2 на чтение\n");
            exit(1);
        }
        char buffer[BUF_SIZE];
        ssize_t bytesRead = read(fifo2_fd, buffer, BUF_SIZE);
        close(fifo2_fd);
        if (bytesRead < 0) {
            fprintf(stderr, "Ошибка чтения из fifo2\n");
            exit(1);
        }
        int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out < 0) {
            fprintf(stderr, "Не удалось открыть выходной файл\n");
            exit(1);
        }
        if (write(fd_out, buffer, bytesRead) != bytesRead) {
            fprintf(stderr, "Ошибка записи в выходной файл\n");
            close(fd_out);
            exit(1);
        }
        close(fd_out);
        exit(0);
    }
    int status;
    waitpid(pid1, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    waitpid(pid2, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    waitpid(pid3, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    unlink(FIFO1);
    unlink(FIFO2);
    printf("Программа выполнена успешно\n");
    return 0;
}
