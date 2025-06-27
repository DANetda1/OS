#!/bin/bash

# Функция, которая выводит сообщение об ошибке
show_error() {
    echo "Ошибка: $1"
}

# Функция, которая проверяет наличие файла и вызывает show_error при отсутствии файла
check_file_exists() {
    file=$1

    if [ ! -f $file ]; then
        show_error "Файл $file не найден"
    else
        echo "Файл $file существует"
    fi
}

# Вызываем функцию check_file_exists с аргументом "example.txt"
check_file_exists "example.txt"
