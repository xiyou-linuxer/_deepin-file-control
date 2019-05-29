#!/bin/bash
cd ./src/Server/;
make;
cd -;
cd ./src/Client/;
make;
cd -;
cd ./src/Hook;
gcc -c Hook.cpp;
ar -crv libHook.a Hook.o;
rm Hook.o;
cd -;


