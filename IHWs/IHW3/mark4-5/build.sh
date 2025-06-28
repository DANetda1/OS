#!/bin/bash

echo "Компиляция server..."
gcc server.c -o server -lpthread

echo "Компиляция doctors..."
gcc doctors.c -o doctors

echo "Компиляция dentist..."
gcc dentist.c -o dentist

echo "Компиляция surgeon..."
gcc surgeon.c -o surgeon

echo "Компиляция therapist..."
gcc therapist.c -o therapist

echo "Компиляция завершена успешно."
