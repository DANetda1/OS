#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

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
        fprintf(stderr, "Значения N1 и N2 должны быть неотрицательными\n");
        return 1;
    }
    if (N2 < N1) {
        fprintf(stderr, "Значение N2 должно быть больше N1\n");
        return 1;
    }

    int fd_in = open(inputFile, O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Не удалось открыть входной файл\n");
        return 1;
    }

    char buffer[5000];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = read(fd_in, buffer, sizeof(buffer));
    close(fd_in);

    if (bytesRead < 0) {
        fprintf(stderr, "Ошибка чтения из входного файла\n");
        return 1;
    }

    if (bytesRead == 0) {
        int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out >= 0) close(fd_out);
        printf("Программа выполнена успешно\n");
        return 0;
    }

    if (N1 >= bytesRead || N2 >= bytesRead) {
        fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
        return 1;
    }

    char fifo1[] = "fifo1";
    char fifo2[] = "fifo2";
    mkfifo(fifo1, 0666);
    mkfifo(fifo2, 0666);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Ошибка при создании процесса\n");
        unlink(fifo1);
        unlink(fifo2);
        return 1;
    }

    if (pid == 0) {
        int fifo_rd = open(fifo1, O_RDONLY);
        if (fifo_rd < 0) {
            exit(1);
        }

        char childBuf[5000];
        memset(childBuf, 0, sizeof(childBuf));
        ssize_t cRead = read(fifo_rd, childBuf, sizeof(childBuf));
        close(fifo_rd);

        if (cRead <= 0) {
            exit(1);
        }

        for (int i = 0; i < (N2 - N1 + 1)/2; i++) {
            char tmp = childBuf[N1 + i];
            childBuf[N1 + i] = childBuf[N2 - i];
            childBuf[N2 - i] = tmp;
        }

        int fifo_wr = open(fifo2, O_WRONLY);
        if (fifo_wr < 0) {
            exit(1);
        }
        if (write(fifo_wr, childBuf, cRead) != cRead) {
            close(fifo_wr);
            exit(1);
        }
        close(fifo_wr);
        exit(0);

    } else {
        int fifo_wr = open(fifo1, O_WRONLY);
        if (fifo_wr < 0) {
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }
        if (write(fifo_wr, buffer, bytesRead) != bytesRead) {
            close(fifo_wr);
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }
        close(fifo_wr);

        int fifo_rd = open(fifo2, O_RDONLY);
        if (fifo_rd < 0) {
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesFromChild = read(fifo_rd, buffer, sizeof(buffer));
        close(fifo_rd);

        if (bytesFromChild < 0) {
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }

        int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out < 0) {
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }
        if (write(fd_out, buffer, bytesFromChild) != bytesFromChild) {
            close(fd_out);
            unlink(fifo1);
            unlink(fifo2);
            return 1;
        }
        close(fd_out);

        int status;
        waitpid(pid, &status, 0);
        unlink(fifo1);
        unlink(fifo2);

        if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
            printf("Программа выполнена успешно\n");
        } else {
            return 1;
        }
    }
    return 0;
}
