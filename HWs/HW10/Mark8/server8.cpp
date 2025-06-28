#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#define BROADCAST_IP "255.255.255.255"
#define PORT 12345

int main() {
    int sock;
    sockaddr_in addr{};
    std::string message;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("Setting broadcast option failed");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    addr.sin_port = htons(PORT);

    std::cout << "[СЕРВЕР] Введите сообщения для рассылки (введите \"exit\" для завершения):\n";

    while (true) {
        std::cout << "[СЕРВЕР] > ";
        std::getline(std::cin, message);

        ssize_t sent = sendto(sock, message.c_str(), message.size(), 0,
                              (sockaddr*)&addr, sizeof(addr));
        if (sent < 0) {
            perror("Send failed");
            break;
        }

        if (message == "exit") {
            std::cout << "[СЕРВЕР] Завершение рассылки.\n";
            break;
        }
    }

    close(sock);
    return 0;
}
