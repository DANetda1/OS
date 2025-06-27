#!/bin/bash

# Функция, которая выводит числа от 1 до N
print_numbers() {
    n=$1 # первый аргумент
    counter=1 # счетчик
# проходимся по циклу и выводим каждое число
    while [ $counter -le $n ]; do
        echo $counter 
        ((counter++))
    done
}

# Функция, которая проверяет, является ли число четным или нечетным
check_even_odd() {
    num=$1

    if [ $((num % 2)) -eq 0 ]; then
        echo "$num - четное число"
    else
        echo "$num - нечетное число"
    fi
}

# Вызываем функции с аргументами
print_numbers 5
check_even_odd 7
