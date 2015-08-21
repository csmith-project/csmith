#!/bin/bash

CSMITH="/home/jacek/Desktop/Imperial/UROP/csmith"
ADAPTER="/home/jacek/Desktop/Imperial/UROP/Adapter/boost_interval_adapter/adaptertracking.o"
TEMPDIR="temp"
COMP="gcc"
TIMEOUT="timeout 30"
RESFILE="wide.txt"

VARSCRIPT="/home/jacek/Desktop/Imperial/UROP/csmith/programs/scripts/wideVars.py"

makeProg(){
  $TIMEOUT ${CSMITH}/src/csmith --seed ${1} --strict-float --float-test --check-global --check-local  > ${TEMPDIR}/prog.c
  return $?
}

compileAndRun(){
  $TIMEOUT gcc ${TEMPDIR}/prog.c -w -c -I${CSMITH}/runtime -DUNSAFE_FLOAT -DFLOAT_TEST_ENABLED -DTRACKING -o ${TEMPDIR}/prog.o &&\
  $TIMEOUT g++ ${TEMPDIR}/prog.o $ADAPTER -o ${TEMPDIR}/prog &&\
  $TIMEOUT ${TEMPDIR}/prog > ${TEMPDIR}/out.txt
  return $?
}

checkWide(){
  $TIMEOUT grep 'Wide' ${TEMPDIR}/out.txt
  return $?
}

findVars(){
  python3 $VARSCRIPT ${TEMPDIR}/out.txt >> "$RESFILE"
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
    echo "$i : " >> "$RESFILE"
    findVars $i
    echo "" >> "$RESFILE"
  else
    echo "singleton $i"
  fi
done
