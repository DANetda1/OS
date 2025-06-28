#!/bin/bash
if [ $# -ne 1 ]; then
  echo "Использование: ./run.sh <кол-во_пациентов>"
  exit 1
fi

cleanup() {
  pkill -TERM doctor9    2>/dev/null
  pkill -TERM registrar9 2>/dev/null
  pkill -TERM controller9 2>/dev/null
  echo -e "\nВсе процессы остановлены"
  exit 0
}
trap cleanup INT

g++ controller9.cpp -o controller9 -lrt -pthread
g++ registrar9.cpp  -o registrar9  -lrt -pthread
g++ doctor9.cpp     -o doctor9     -lrt -pthread

./controller9 "$1" &
CTRL=$!
sleep 1

per=$(( $1 / 2 ))
extra=$(( $1 % 2 ))
./registrar9 1 $per        &
./registrar9 2 $((per+extra)) &
./doctor9 0 &
./doctor9 1 &
./doctor9 2 &

wait $CTRL
cleanup
