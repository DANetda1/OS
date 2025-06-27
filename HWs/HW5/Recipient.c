#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

// PID отправителя
int PID_sender_;
// Переменная для итогового числа
int data_ = 0;
// Какой по счету бит передается
int count_bits = -1;
// Указывает на знак числа
bool flag_positive = true;

// В случае когда получаемый бит 0 ничего не делаем
void Action1(int signal) {
    // 1ая операция для установления знака числа
    if (count_bits == -1) {
        flag_positive = true;
    }
    //Увеличиваем номер текущего бита
    ++count_bits;
    // Ответ Sender, чтобы он мог продолжить работу
    kill(PID_sender_, SIGUSR1);
}

// В случае когда получаемый бит 1 прибавляем нужную степень двойки
void Action2(int signal) {
    // 1ая операция для установления знака числа
    if (count_bits == -1) {
        flag_positive = false;
    } else {
        data_ += pow(2, count_bits);
    }
    //Увеличиваем номер текущего бита
    ++count_bits;
    // Ответ Sender, чтобы он мог продолжить работу
    kill(PID_sender_, SIGUSR1);
}
// В случае получения сигнала SIGINT приостанавливаем работу, то есть мы получили все число и присылаем сигнал Sender,
// чтобы он тоже закончил исполнение
void Stop(int signal) {
    printf("Recievent number : %d", data_ * flag_positive + data_ * (flag_positive - 1));
    kill(PID_sender_, SIGUSR2);
    exit(0);
}


int main() {
    // Связываем сигналы и их обработчики
    signal(SIGUSR1, Action1);
    signal(SIGUSR2, Action2);
    signal(SIGINT, Stop);

    printf("I am Recipient, take my PID - %d. So, write PID of Sender in order to receive number from him\n", getpid());

    printf("Input PID of Sender: ");
    scanf("%d", &PID_sender_);

    while (true);
}
