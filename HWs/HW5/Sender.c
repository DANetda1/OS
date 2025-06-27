#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
// PID получателя
int PID_recipient_;
// Обрабатываемое число
int data_;

// Отправка очередного бита через нужный сигнал
void send_bit(bool bit, int address) {
    if (bit) kill(address, SIGUSR1);
    else kill(address, SIGUSR2);
}
// Обработчик для отправки сигнала
void Action(int signal) {
    if (data_ == 0) kill(PID_recipient_, SIGINT);
    else if (data_ % 2 == 0) send_bit(true, PID_recipient_);
    else send_bit(false, PID_recipient_);
    data_ /= 2;
}
// Обработчик для остановки исполнения sender
void Stop(int signal) {
    printf("Number was sent");
    exit(0);
}

int main() {
    // Присваиваем обработчики сигналам
    signal(SIGUSR1, Action);
    signal(SIGUSR2, Stop);

    printf("I am Sender, take my PID - %d. So, write PID of Recipient and number in order to send this number\n",
           getpid());

    printf("Input PID of Recipient: ");
    scanf("%d", &PID_recipient_);

    printf("Input number, which you want to send: ");
    scanf("%d", &data_);

    send_bit(data_ >= 0, PID_recipient_);
    while (true);
}
