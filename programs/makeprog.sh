#!/bin/bash
../src/csmith --seed ${1} --strict-float --float-test --check-global >prog.c
gcc prog.c -w -c -I../runtime -o prog1.o
gcc prog.c -w -c -I../runtime -DUNSAFE_FLOAT -DFLOAT_TEST_ENABLED -o prog2.o
g++ prog1.o ../../Adapter/boost_interval_adapter/adapter.o -o prog1
g++ prog2.o ../../Adapter/boost_interval_adapter/adapter.o -o prog2
rm *.o
./prog1 > out1.txt
./prog2 > out2.txt
