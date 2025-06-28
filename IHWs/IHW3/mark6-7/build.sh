#!/bin/bash

echo "Компиляция серверной части..."
gcc server.c -o server -lpthread

echo "Компиляция клиента doctors..."
gcc doctors.c -o doctors

echo "Компиляция специалиста dentist..."
gcc dentist.c -o dentist

echo "Компиляция специалиста surgeon..."
gcc surgeon.c -o surgeon

echo "Компиляция специалиста therapist..."
gcc therapist.c -o therapist

echo "Компиляция мониторинга monitor..."
gcc monitor.c -o monitor

echo "Все файлы успешно скомпилированы."
