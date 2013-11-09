#!/bin/bash

ulimit -t 120

${CSMITH_HOME}/driver/check_unique.pl XX_CRASHFILE XX_DIR XX_OPT "XX_COMMAND"

gcc -w -c -O0 small.c &&\
! XX_COMMAND XX_OPT small.c -c -w > out.txt 2>&1 &&\
grep XX_STRING out.txt
