#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define DOCTOR_PORT_STOMATOLOG 12346
#define DOCTOR_PORT_KHIRURG 12347
#define DOCTOR_PORT_TERAPEVT 12348

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr, doctoraddr;
    char buffer[MAXLINE];

    if (argc != 2) {
        printf("Использование: %s <порт_сервера>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    memset(&doctoraddr, 0, sizeof(doctoraddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Ошибка привязки сокета");
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен на порту %d\n", port);

    while (1) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, MAXLINE-1, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("Ошибка при получении");
            continue;
        }
        buffer[n] = '\0';
        printf("Пациент поступил: %s\n", buffer);

        int target_port = 0;
        if (strstr(buffer, "Стоматолог") != NULL) {
            target_port = DOCTOR_PORT_STOMATOLOG;
            printf("Направление: Стоматолог\n");
        } else if (strstr(buffer, "Хирург") != NULL) {
            target_port = DOCTOR_PORT_KHIRURG;
            printf("Направление: Хирург\n");
        } else if (strstr(buffer, "Терапевт") != NULL) {
            target_port = DOCTOR_PORT_TERAPEVT;
            printf("Направление: Терапевт\n");
        } else {
            printf("Не удалось определить врача\n");
            continue;
        }

        doctoraddr.sin_family = AF_INET;
        doctoraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        doctoraddr.sin_port = htons(target_port);

        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&doctoraddr, sizeof(doctoraddr));
    }

    close(sockfd);
    return 0;
}
