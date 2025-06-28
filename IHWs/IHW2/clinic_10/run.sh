#!/bin/bash
if [ $# -ne 1 ]; then echo "usage: ./run.sh N"; exit 1; fi
cleanup(){ pkill -TERM doctor10 2>/dev/null; pkill -TERM registrar10 2>/dev/null; pkill -TERM controller10 2>/dev/null; echo; exit 0; }
trap cleanup INT
g++ controller10.cpp -o controller10
g++ registrar10.cpp  -o registrar10
g++ doctor10.cpp     -o doctor10
./controller10 "$1" &
sleep 1
half=$(( $1 / 2 )); rest=$(( $1 - half ))
./registrar10 1 $half &
./registrar10 2 $rest &
./doctor10 0 & ./doctor10 1 & ./doctor10 2 &
wait
