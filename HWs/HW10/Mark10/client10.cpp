#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#define MULTICAST_GROUP "239.1.2.3"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock;
    sockaddr_in addr{};
    ip_mreq mreq{};
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR error");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Joining multicast group error");
        return 1;
    }

    std::cout << "[КЛИЕНТ] Ожидаю сообщения...\n";

    while (true) {
        socklen_t addrlen = sizeof(addr);
        ssize_t nbytes = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                  (sockaddr*)&addr, &addrlen);
        if (nbytes < 0) {
            perror("Receive failed");
            break;
        }

        buffer[nbytes] = '\0';
        std::string msg(buffer);
        std::cout << "[КЛИЕНТ] Получено: " << msg << "\n";

        if (msg == "exit") {
            std::cout << "[КЛИЕНТ] Получено сообщение о завершении. Выход...\n";
            break;
        }
    }

    close(sock);
    return 0;
}
