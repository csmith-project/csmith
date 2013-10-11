#!/bin/bash

# ${CSMITH_HOME}/driver/check_unique.pl out.txt &&\

! XX_COMMAND small.c -w > out.txt 2>&1 &&\
grep XX_STRING out.txt
