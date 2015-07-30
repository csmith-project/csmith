#!/bin/bash

COMPILER="gcc"
CSMITH_PATH="/home/jacek/Desktop/Imperial/UROP/csmith/"
ADAPTER_OBJECT_PATH="/home/jacek/Desktop/Imperial/UROP/Adapter/boost_interval_adapter/adapter.o"
GEN_ERROR_FILE="genError.txt"
COMP_ERROR_FILE="compError.txt"
RUN_ERROR_FILE="runError.txt"
TIMEOUT_MACRO="timeout 60 "
TEMP_DIR="temp"
RESULTS_DIR="results"
GEN_ERROR_FILE="${RESULTS_DIR}/genError.txt"
COMP_ERROR_FILE="${RESULTS_DIR}/compError.txt"
RUN_ERROR_FILE="${RESULTS_DIR}/runError.txt"
RESULT_ERROR_FILE="${RESULTS_DIR}/resError.txt"
WIDE_COUNT_FILE="${RESULTS_DIR}/wideCount.txt"

makeProg(){
  ${TIMEOUT_MACRO} ${CSMITH_PATH}src/csmith --seed ${1} --strict-float --float-test --check-global > ${TEMP_DIR}/prog.c
  return $?
}

compileInFloatTestMode(){
  ${TIMEOUT_MACRO} ${COMPILER} ${TEMP_DIR}/prog.c -O0 -w -c -I${CSMITH_PATH}runtime -DUNSAFE_FLOAT -DFLOAT_TEST_ENABLED -o ${TEMP_DIR}/progFloatTestMode.o && \
  ${TIMEOUT_MACRO} g++ ${TEMP_DIR}/progFloatTestMode.o ${ADAPTER_OBJECT_PATH} -o ${TEMP_DIR}/progFloatTestMode
  return $?
}

runInFloatTestMode(){
  ${TIMEOUT_MACRO} ${TEMP_DIR}/progFloatTestMode > ${TEMP_DIR}/outFloatTestMode.txt
  return $?
}

compileInNormalMode(){
  ${TIMEOUT_MACRO} ${COMPILER} ${TEMP_DIR}/prog.c -O1 -w -c -I${CSMITH_PATH}runtime -DUNSAFE_FLOAT -o ${TEMP_DIR}/progNormalMode1.o && \
  ${TIMEOUT_MACRO} g++ ${TEMP_DIR}/progNormalMode1.o ${ADAPTER_OBJECT_PATH} -o ${TEMP_DIR}/progNormal1 && \
  ${TIMEOUT_MACRO} ${COMPILER} ${TEMP_DIR}/prog.c -O2 -w -c -I${CSMITH_PATH}runtime -DUNSAFE_FLOAT -o ${TEMP_DIR}/progNormalMode2.o && \
  ${TIMEOUT_MACRO} g++ ${TEMP_DIR}/progNormalMode2.o ${ADAPTER_OBJECT_PATH} -o ${TEMP_DIR}/progNormal2 && \
  ${TIMEOUT_MACRO} ${COMPILER} ${TEMP_DIR}/prog.c -O3 -w -c -I${CSMITH_PATH}runtime -DUNSAFE_FLOAT -o ${TEMP_DIR}/progNormalMode3.o && \
  ${TIMEOUT_MACRO} g++ ${TEMP_DIR}/progNormalMode3.o ${ADAPTER_OBJECT_PATH} -o ${TEMP_DIR}/progNormal3
  return $?
}

runInNormalMode(){
  ${TIMEOUT_MACRO} ${TEMP_DIR}/progNormal1 > ${TEMP_DIR}/outNormalMode1.txt && \
  ${TIMEOUT_MACRO} ${TEMP_DIR}/progNormal2 > ${TEMP_DIR}/outNormalMode2.txt && \
  ${TIMEOUT_MACRO} ${TEMP_DIR}/progNormal3 > ${TEMP_DIR}/outNormalMode3.txt
  return $?
}

validateResults(){
  python3 validate.py ${TEMP_DIR}/outNormalMode1.txt ${TEMP_DIR}/outFloatTestMode.txt >> "${WIDE_COUNT_FILE}" && \
  python3 validate.py ${TEMP_DIR}/outNormalMode2.txt ${TEMP_DIR}/outFloatTestMode.txt >> "${WIDE_COUNT_FILE}" && \
  python3 validate.py ${TEMP_DIR}/outNormalMode3.txt ${TEMP_DIR}/outFloatTestMode.txt >> "${WIDE_COUNT_FILE}"
  return $?
}


checkpoint(){
  if [ $1 -ne 0 ]; then
    echo "$2" >> "$3"
    return 1
  fi
  return 0
}


testSeed(){
  makeProg $1
  checkpoint $? $1 $GEN_ERROR_FILE || return 1
  compileInFloatTestMode 
  checkpoint $? $1 $COMP_ERROR_FILE || return 1
  runInFloatTestMode
  checkpoint $? $1 $RUN_ERROR_FILE || return 1
  compileInNormalMode
  checkpoint $? $1 $COMP_ERROR_FILE || return 1
  runInNormalMode
  checkpoint $? $1 $RUN_ERROR_FILE || return 1
  validateResults
  checkpoint $? $1 $RESULT_ERROR_FILE || return 1
  return 0
}




mkdir -p $TEMP_DIR
mkdir -p $RESULTS_DIR
touch ${GEN_ERROR_FILE}
touch ${COMP_ERROR_FILE}
touch ${RUN_ERROR_FILE}
touch ${RESULT_ERROR_FILE}
touch ${WIDE_COUNT_FILE}

for seed in $(seq $1 $2); do
  echo "$seed"
  if testSeed $seed; then
    echo 'OK'
  else
    echo 'ERROR'
  fi
done
