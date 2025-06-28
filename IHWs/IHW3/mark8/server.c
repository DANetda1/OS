#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_MONITORS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sock;
    struct sockaddr_in addr;
} client_t;

client_t doctors[MAX_CLIENTS];
int doctor_count = 0;

client_t specialists[3];
char* specialist_names[3] = {"Стоматолог", "Хирург", "Терапевт"};
int specialist_count = 0;

client_t monitors[MAX_MONITORS];
int monitor_count = 0;

int specialist_counter[3] = {0, 0, 0};

int served_patients = 0;
int shutdown_flag = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t doctor_threads[MAX_CLIENTS];

void send_to_all_monitors(const char* message) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s\n", message);
    for (int i = 0; i < monitor_count; i++) {
        if (send(monitors[i].sock, buffer, strlen(buffer), 0) <= 0) {
            close(monitors[i].sock);
            for (int j = i; j < monitor_count - 1; j++) {
                monitors[j] = monitors[j + 1];
            }
            monitor_count--;
            i--;
        }
    }
}

void* handle_doctor(void* arg) {
    int sock = *(int*)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recv(sock, buffer, BUFFER_SIZE, 0);
        if (len <= 0) break;

        send_to_all_monitors(buffer);

        char target[32];
        int patient;
        sscanf(buffer, "Пациент #%d запросил врача: %s", &patient, target);

        for (int i = 0; i < 3; i++) {
            if (strstr(target, specialist_names[i])) {
                char msg[BUFFER_SIZE];
                snprintf(msg, sizeof(msg), "Доктор направил Пациента #%d к врачу: %s", patient, specialist_names[i]);
                send_to_all_monitors(msg);

                snprintf(msg, sizeof(msg), "%s начал лечение Пациента #%d", specialist_names[i], patient);
                send(specialists[i].sock, msg, strlen(msg), 0);
                send_to_all_monitors(msg);
                sleep(2);
                snprintf(msg, sizeof(msg), "%s закончил лечение Пациента #%d", specialist_names[i], patient);
                send(specialists[i].sock, msg, strlen(msg), 0);
                send(sock, msg, strlen(msg), 0);
                send_to_all_monitors(msg);
                pthread_mutex_lock(&lock);
                specialist_counter[i]++;
                pthread_mutex_unlock(&lock);
                break;
            }
        }

        pthread_mutex_lock(&lock);
        served_patients++;
        if (served_patients == 10 && !shutdown_flag) {
            shutdown_flag = 1;
            for (int j = 0; j < 3; j++) {
                send(specialists[j].sock, "Больница закрывается", 44, 0);
                close(specialists[j].sock);
            }
            for (int j = 0; j < doctor_count; j++) {
                send(doctors[j].sock, "Больница закрывается", 44, 0);
            }
            char summary[BUFFER_SIZE];
            for (int i = 0; i < 3; i++) {
                snprintf(summary, sizeof(summary), "Врач %s сегодня принял: %d пациентов", specialist_names[i], specialist_counter[i]);
                send_to_all_monitors(summary);
            }
            send_to_all_monitors("Больница завершила свою работу");
            for (int i = 0; i < monitor_count; i++) {
                close(monitors[i].sock);
            }
        }
        pthread_mutex_unlock(&lock);
    }

    close(sock);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Использование: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 10);

    while (doctor_count < 6 || monitor_count < MAX_MONITORS) {
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "SPECIALIST", 10) == 0) {
            specialists[specialist_count].sock = client_sock;
            specialists[specialist_count].addr = client_addr;
            specialist_count++;
        } else if (strncmp(buffer, "MONITOR", 7) == 0) {
            if (monitor_count < MAX_MONITORS) {
                monitors[monitor_count].sock = client_sock;
                monitors[monitor_count].addr = client_addr;
                monitor_count++;
            }
        } else {
            doctors[doctor_count].sock = client_sock;
            doctors[doctor_count].addr = client_addr;
            pthread_create(&doctor_threads[doctor_count], NULL, handle_doctor, &client_sock);
            doctor_count++;
        }

        if (doctor_count >= 6 && shutdown_flag) {
            break;
        }
    }

    while (!shutdown_flag) {
        sleep(1);
    }

    for (int i = 0; i < doctor_count; i++) {
        pthread_join(doctor_threads[i], NULL);
        close(doctors[i].sock);
    }

    close(server_sock);
    return 0;
}
