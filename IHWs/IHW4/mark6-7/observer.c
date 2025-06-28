#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(12349);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Ошибка привязки");
        exit(EXIT_FAILURE);
    }

    printf("Наблюдатель запущен на порту 12349\n");

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
