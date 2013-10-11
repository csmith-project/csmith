#!/bin/bash

! XX_COMMAND small.c -w > out.txt 2>&1 &&\
${CSMITH_HOME}/driver/check_unique.pl out.txt &&\
grep XX_STRING out.txt
