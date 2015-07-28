#!/bin/bash

ADAPTER_PATH='/home/jacek/Desktop/Imperial/UROP/Adapter/boost_interval_adapter/adapter.o'
RUNTIME_PATH='/home/jacek/Desktop/Imperial/UROP/csmith/runtime'
CSMITH_PATH='/home/jacek/Desktop/Imperial/UROP/csmith/src/csmith'

for i in $(seq ${1} ${2}); do
  ${CSMITH_PATH} --seed $i --float-test --check-global >prog.c
  gcc prog.c -I${RUNTIME_PATH} -c -o prog.o > err.txt 2>&1
  g++ prog.o ${ADAPTER_PATH} -o prog
  if grep 'incompatible' err.txt; then
    echo "ERROR $i"
  else
    echo "OK $i"
  fi
done
