#!/bin/bash

echo "Компиляция server.c..."
gcc server.c -o server

echo "Компиляция doctor.c..."
gcc doctor.c -o doctor

echo "Компиляция client.c..."
gcc client.c -o client

echo "Компиляция завершена!"
