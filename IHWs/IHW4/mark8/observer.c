#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];

    if (argc != 3) {
        printf("Использование: %s <ip_сервера> <порт_сервера>\n", argv[0]);
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

    sendto(sockfd, "Я наблюдатель", strlen("Я наблюдатель"), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Наблюдатель подключён к серверу %s:%d\n", server_ip, port);

    while (1) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, MAXLINE-1, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("Ошибка при получении");
            continue;
        }
        buffer[n] = '\0';
        printf("Наблюдатель: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
