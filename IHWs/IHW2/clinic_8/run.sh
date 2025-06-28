if [ $# -ne 1 ]; then
  echo "Использование: ./run.sh <кол-во_пациентов>"
  exit 1
fi

pkill controller
pkill registrar
pkill doctor
ipcrm -M 1234 2>/dev/null
ipcrm -S 5678 2>/dev/null

g++ controller.cpp -o controller
g++ registrar.cpp -o registrar
g++ doctor.cpp -o doctor

./controller "$1" > controller.log 2>&1 &
sleep 2
clear

./registrar 1 &
./registrar 2 &
./doctor 0 &
./doctor 1 &
./doctor 2 &
