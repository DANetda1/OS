#!/bin/bash

# Функция, которая выводит приветствие
say_hello() {
    echo "Привет, $1!"
}

# Функция, которая вызывает say_hello
greet() {
    name=$1
    say_hello $name
}

# Вызываем функцию greet с аргументом "Мир"
greet "Мир"
