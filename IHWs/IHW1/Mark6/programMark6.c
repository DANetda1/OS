#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#define BUF_SIZE 5000

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
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        fprintf(stderr, "Ошибка создания pipe\n");
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Ошибка fork\n");
        return 1;
    }
    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        char buffer[BUF_SIZE];
        ssize_t bytesRead = read(pipe1[0], buffer, BUF_SIZE);
        close(pipe1[0]);
        if (bytesRead < 0) {
            fprintf(stderr, "Ошибка чтения из pipe1\n");
            exit(1);
        }
        if (N2 >= bytesRead) {
            fprintf(stderr, "Значения N1 и N2 выходят за пределы размера данных\n");
            exit(1);
        }
        reverse_substring(buffer, N1, N2);
        if (write(pipe2[1], buffer, bytesRead) != bytesRead) {
            fprintf(stderr, "Ошибка записи в pipe2\n");
            exit(1);
        }
        close(pipe2[1]);
        exit(0);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);
        int fd_in = open(inputFile, O_RDONLY);
        if (fd_in < 0) {
            fprintf(stderr, "Не удалось открыть входной файл\n");
            exit(1);
        }
        char buffer[BUF_SIZE];
        ssize_t bytesRead = read(fd_in, buffer, BUF_SIZE);
        close(fd_in);
        if (bytesRead < 0) {
            fprintf(stderr, "Ошибка чтения входного файла\n");
            exit(1);
        }
        if (write(pipe1[1], buffer, bytesRead) != bytesRead) {
            fprintf(stderr, "Ошибка записи в pipe1\n");
            exit(1);
        }
        close(pipe1[1]);
        ssize_t bytesFromChild = read(pipe2[0], buffer, BUF_SIZE);
        close(pipe2[0]);
        if (bytesFromChild < 0) {
            fprintf(stderr, "Ошибка чтения из pipe2\n");
            exit(1);
        }
        int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out < 0) {
            fprintf(stderr, "Не удалось открыть выходной файл\n");
            exit(1);
        }
        if (write(fd_out, buffer, bytesFromChild) != bytesFromChild) {
            fprintf(stderr, "Ошибка записи в выходной файл\n");
            close(fd_out);
            exit(1);
        }
        close(fd_out);
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            exit(1);
        }
        printf("Программа выполнена успешно\n");
    }
    return 0;
}
