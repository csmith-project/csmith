#!/bin/bash

CSMITH="/home/jacek/Desktop/Imperial/UROP/csmith"
ADAPTER="/home/jacek/Desktop/Imperial/UROP/Adapter/boost_interval_adapter/adaptertracking.o"
TEMPDIR="temp"
COMP="gcc"
TIMEOUT="timeout 30"
RESFILE="wide.txt"

makeProg(){
  $TIMEOUT ${CSMITH}/src/csmith --seed ${1} --strict-float --float-test > ${TEMPDIR}/prog.c
  return $?
}

compileAndRun(){
  $TIMEOUT gcc ${TEMPDIR}/prog.c -w -c -I${CSMITH}/runtime -DUNSAFE_FLOAT -DFLOAT_TEST_ENABLED -DTRACKING -DWIDE -o ${TEMPDIR}/prog1.o &&\
  $TIMEOUT g++ ${TEMPDIR}/prog1.o $ADAPTER -o ${TEMPDIR}/prog1 &&\
  $TIMEOUT ${TEMPDIR}/prog1 > ${TEMPDIR}/out1.txt
  return $?
}

checkWide(){
  $TIMEOUT grep 'Wide' ${TEMPDIR}/out1.txt
  return $?
}

mkdir -p $TEMPDIR
touch $RESFILE

for i in $(seq $1 $2); do
  makeProg $i &&\
  compileAndRun
  if [ $? -ne 0 ]; then
    continue
  fi
  checkWide
  if [ $? -eq 0 ]; then
    echo "wide $i"
    echo "$i" >> "$RESFILE"
  else
    echo "singleton $i"
  fi
done
