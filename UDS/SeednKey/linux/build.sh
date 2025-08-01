g++ -Wall -ansi -pedantic -shared -fPIC -o libseednkey.so seednkey.c
g++ main.cpp -o main -ldl
./main