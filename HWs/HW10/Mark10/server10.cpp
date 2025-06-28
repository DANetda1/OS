#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#define MULTICAST_GROUP "239.1.2.3"
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

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
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
