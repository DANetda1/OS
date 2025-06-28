#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAXLINE 1024
#define DOCTOR_PORT_STOMATOLOG 12346
#define DOCTOR_PORT_KHIRURG 12347
#define DOCTOR_PORT_TERAPEVT 12348
#define MAX_OBSERVERS 10
#define MAX_DOCTORS 3

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
} Observer;

int sockfd;
Observer observers[MAX_OBSERVERS];
int observer_count = 0;

void finish(int sig) {
    char msg[] = "Завершение работы";
    for (int i = 0; i < observer_count; i++) {
        sendto(sockfd, msg, strlen(msg), 0, (const struct sockaddr *)&observers[i].addr, observers[i].addr_len);
    }

    struct sockaddr_in doctoraddr;
    doctoraddr.sin_family = AF_INET;
    doctoraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    doctoraddr.sin_port = htons(DOCTOR_PORT_STOMATOLOG);
    sendto(sockfd, msg, strlen(msg), 0, (const struct sockaddr *)&doctoraddr, sizeof(doctoraddr));

    doctoraddr.sin_port = htons(DOCTOR_PORT_KHIRURG);
    sendto(sockfd, msg, strlen(msg), 0, (const struct sockaddr *)&doctoraddr, sizeof(doctoraddr));

    doctoraddr.sin_port = htons(DOCTOR_PORT_TERAPEVT);
    sendto(sockfd, msg, strlen(msg), 0, (const struct sockaddr *)&doctoraddr, sizeof(doctoraddr));

    printf("Сервер завершает работу. Всем клиентам отправлено сообщение о завершении.\n");
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
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

    signal(SIGINT, finish);

    printf("Сервер запущен на порту %d\n", port);

    while (1) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, MAXLINE-1, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("Ошибка при получении");
            continue;
        }
        buffer[n] = '\0';

        if (strcmp(buffer, "Я наблюдатель") == 0) {
            if (observer_count < MAX_OBSERVERS) {
                observers[observer_count].addr = cliaddr;
                observers[observer_count].addr_len = len;
                observer_count++;
                printf("Подключился новый наблюдатель. Всего: %d\n", observer_count);
            }
            continue;
        }

        char message[MAXLINE];
        snprintf(message, sizeof(message), "Пациент поступил: %s", buffer);
        printf("%s\n", message);
        for (int i = 0; i < observer_count; i++) {
            sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&observers[i].addr, observers[i].addr_len);
        }

        int target_port = 0;
        if (strstr(buffer, "Стоматолог") != NULL) {
            target_port = DOCTOR_PORT_STOMATOLOG;
            snprintf(message, sizeof(message), "Направление: Стоматолог");
        } else if (strstr(buffer, "Хирург") != NULL) {
            target_port = DOCTOR_PORT_KHIRURG;
            snprintf(message, sizeof(message), "Направление: Хирург");
        } else if (strstr(buffer, "Терапевт") != NULL) {
            target_port = DOCTOR_PORT_TERAPEVT;
            snprintf(message, sizeof(message), "Направление: Терапевт");
        } else {
            snprintf(message, sizeof(message), "Не удалось определить врача");
        }

        printf("%s\n", message);
        for (int i = 0; i < observer_count; i++) {
            sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&observers[i].addr, observers[i].addr_len);
        }

        if (target_port != 0) {
            doctoraddr.sin_family = AF_INET;
            doctoraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            doctoraddr.sin_port = htons(target_port);
            sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&doctoraddr, sizeof(doctoraddr));
        }
    }

    return 0;
}
