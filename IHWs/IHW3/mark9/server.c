#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 20
#define BUFFER_SIZE 1024
#define MAX_QUEUE 100

typedef struct {
    int sock;
    struct sockaddr_in addr;
} client_t;

typedef struct {
    int patient_id;
    int specialist_id;
} patient_t;

client_t monitors[MAX_CLIENTS];
int monitor_count = 0;

client_t specialists[3];
int specialist_online[3] = {0, 0, 0};

char* specialist_names[3] = {"Стоматолог", "Хирург", "Терапевт"};
int specialist_counter[3] = {0};

patient_t patient_queue[MAX_QUEUE];
int queue_size = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int total_patients = 0;
int completed_patients = 0;
int shutdown_flag = 0;

void send_to_monitors(const char* message) {
    char formatted[BUFFER_SIZE];
    snprintf(formatted, sizeof(formatted), "%s\n", message);
    for (int i = 0; i < monitor_count; i++) {
        if (send(monitors[i].sock, formatted, strlen(formatted), 0) <= 0) {
            close(monitors[i].sock);
            for (int j = i; j < monitor_count - 1; j++)
                monitors[j] = monitors[j + 1];
            monitor_count--;
            i--;
        }
    }
}

void process_queue() {
    for (int i = 0; i < queue_size;) {
        int sid = patient_queue[i].specialist_id;
        if (specialist_online[sid]) {
            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s начал лечение Пациента #%d", specialist_names[sid], patient_queue[i].patient_id);
            send(specialists[sid].sock, msg, strlen(msg), 0);
            send_to_monitors(msg);
            sleep(2);
            snprintf(msg, sizeof(msg), "%s закончил лечение Пациента #%d", specialist_names[sid], patient_queue[i].patient_id);
            send(specialists[sid].sock, msg, strlen(msg), 0);
            send_to_monitors(msg);
            specialist_counter[sid]++;
            completed_patients++;
            for (int j = i; j < queue_size - 1; j++)
                patient_queue[j] = patient_queue[j + 1];
            queue_size--;
        } else {
            i++;
        }
    }
}

void handle_request(char* buffer) {
    int id;
    char spec[32];
    sscanf(buffer, "Пациент #%d запросил врача: %s", &id, spec);
    int target = -1;
    for (int i = 0; i < 3; i++) {
        if (strstr(spec, specialist_names[i])) {
            target = i;
            break;
        }
    }
    if (target == -1) return;

    char msg[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "Доктор направил Пациента #%d к врачу: %s", id, specialist_names[target]);
    send_to_monitors(buffer);
    send_to_monitors(msg);

    pthread_mutex_lock(&lock);
    patient_queue[queue_size].patient_id = id;
    patient_queue[queue_size].specialist_id = target;
    queue_size++;
    total_patients++;
    process_queue();
    pthread_mutex_unlock(&lock);
}

void* handle_client(void* arg) {
    int sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);

    if (strncmp(buffer, "MONITOR", 7) == 0) {
        pthread_mutex_lock(&lock);
        monitors[monitor_count].sock = sock;
        monitor_count++;
        pthread_mutex_unlock(&lock);
        while (recv(sock, buffer, BUFFER_SIZE, 0) > 0) {}
        close(sock);
        return NULL;
    }

    if (strncmp(buffer, "SPECIALIST:", 11) == 0) {
        int id = -1;
        if (strstr(buffer, "DENTIST")) id = 0;
        else if (strstr(buffer, "SURGEON")) id = 1;
        else if (strstr(buffer, "THERAPIST")) id = 2;
        if (id != -1) {
            pthread_mutex_lock(&lock);
            specialists[id].sock = sock;
            specialist_online[id] = 1;
            pthread_mutex_unlock(&lock);
            process_queue();
            while (recv(sock, buffer, BUFFER_SIZE, 0) > 0) {}
            pthread_mutex_lock(&lock);
            specialist_online[id] = 0;
            pthread_mutex_unlock(&lock);
            close(sock);
            return NULL;
        }
    }

    pthread_mutex_lock(&lock);
    while (recv(sock, buffer, BUFFER_SIZE, 0) > 0) {
        handle_request(buffer);
        if (completed_patients == 10 && !shutdown_flag) {
            shutdown_flag = 1;
            char msg[BUFFER_SIZE];
            for (int i = 0; i < 3; i++) {
                snprintf(msg, sizeof(msg), "Врач %s сегодня принял: %d пациентов", specialist_names[i], specialist_counter[i]);
                send_to_monitors(msg);
            }
            send_to_monitors("Больница завершила свою работу");
        }
    }
    pthread_mutex_unlock(&lock);
    close(sock);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Использование: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr, cli;
    socklen_t len = sizeof(cli);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);

    while (1) {
        int* sock_ptr = malloc(sizeof(int));
        *sock_ptr = accept(server_sock, (struct sockaddr*)&cli, &len);
        pthread_t t;
        pthread_create(&t, NULL, handle_client, sock_ptr);
        pthread_detach(t);
    }

    close(server_sock);
    return 0;
}
