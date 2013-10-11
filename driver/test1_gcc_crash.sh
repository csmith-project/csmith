#!/bin/bash

gcc -w -c -O0 small.c &&\
! XX_COMMAND small.c -w > out.txt 2>&1 &&\
${CSMITH_HOME}/driver/check_unique.pl XX_CRASHFILE out.txt XX_DIR &&\
grep XX_STRING out.txt
