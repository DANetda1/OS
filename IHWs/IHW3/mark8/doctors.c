#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

char* specialties[] = {"Стоматолог", "Хирург", "Терапевт"};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Использование: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    send(sock, "DOCTOR", 6, 0);

    srand(time(NULL));
    char buffer[BUFFER_SIZE];
    for (int i = 1; i <= 10; i++) {
        sleep(1);
        int idx = rand() % 3;
        snprintf(buffer, sizeof(buffer), "Пациент #%d запросил врача: %s", i, specialties[idx]);
        printf("%s\n", buffer);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("Мы выписали вам направление к нужному врачу\n");
        printf("Следующий пациент, пожалуйста\n");
    }

    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("Больница закрывается, Доктора уходят домой.\n");

    close(sock);
    return 0;
}
