#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

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
    send(sock, "SPECIALIST:DENTIST", 19, 0);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        printf("%s\n", buffer);
        if (strstr(buffer, "Больница закрывается")) {
            printf("Больница закрывается, Стоматолог уходит домой.\n");
            break;
        }
    }

    close(sock);
    return 0;
}
