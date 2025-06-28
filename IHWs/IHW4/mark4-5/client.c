#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE 1024

const char* doctors[] = {"Стоматолог", "Хирург", "Терапевт"};

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAXLINE];

    if (argc != 3) {
        printf("Использование: %s <ip_сервера> <порт>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* server_ip = argv[1];
    int port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    srand(time(NULL));

    for (int i = 1; i <= 10; i++) {
        int idx = rand() % 3;
        snprintf(buffer, sizeof(buffer), "Пациент #%d записался на прием к врачу %s", i, doctors[idx]);

        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        printf("%s\n", buffer);
        printf("Мы выписали вам направление к нужному врачу\n");
        printf("Следующий пациент, пожалуйста\n");

        sleep(1);
    }

    close(sockfd);
    return 0;
}
