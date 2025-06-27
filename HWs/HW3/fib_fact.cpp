#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <dirent.h>
#include <limits>
#include <cstdint>
#include <locale>

using namespace std;

uint64_t fibonacci(uint64_t n) {
    uint64_t a = 0, b = 1, c;
    for (uint64_t i = 2; i <= n; ++i) {
        if (b > numeric_limits<uint64_t>::max() - a) {
            cerr << "Переполнение Фибоначчи!" << endl;
            exit(1);
        }
        c = a + b;
        a = b;
        b = c;
    }
    return n ? b : a;
}

uint64_t factorial(uint64_t n) {
    uint64_t result = 1;
    for (uint64_t i = 2; i <= n; ++i) {
        if (result > numeric_limits<uint64_t>::max() / i) {
            cerr << "Переполнение факториала!" << endl;
            exit(1);
        }
        result *= i;
    }
    return result;
}

void list_directory() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            cout << ent->d_name << endl;
        }
        closedir(dir);
    } else {
        perror("opendir");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    if (argc != 2) {
        cerr << "Использование: " << argv[0] << " <число>" << endl;
        return 1;
    }
    uint64_t num = strtoull(argv[1], nullptr, 10);
    
    cout << "Родительский процесс (PID: " << getpid() << ", Родительский PID: " << getppid() << ")" << endl;
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        cout << "Дочерний процесс (PID: " << getpid() << ", Родительский PID: " << getppid() << ")" << endl;
        cout << "Факториал(" << num << ") = " << factorial(num) << endl;
        return 0;
    } else {
        cout << "Родительский процесс создал дочерний (PID: " << pid << ")" << endl;
        cout << "Фибоначчи(" << num << ") = " << fibonacci(num) << endl;
        wait(nullptr);
        
        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("fork");
            return 1;
        }
        if (pid2 == 0) {
            cout << "Процесс вывода содержимого каталога (PID: " << getpid() << ", Родительский PID: " << getppid() << ")" << endl;
            list_directory();
            return 0;
        } else {
            wait(nullptr);
        }
    }
    return 0;
}