#!/bin/bash

ulimit -t 120

${CSMITH_HOME}/driver/check_unique.pl XX_CRASHFILE XX_DIR XX_OPT "XX_COMMAND"

gcc -c small.c > gcc.out 2>&1 &&\
! grep 'data definition has no type or storage class' gcc.out &&\
! grep 'assumed to have one element' gcc.out &&\
! grep 'control reaches end of non-void function' gcc.out &&\
! grep 'return type defaults to' gcc.out &&\
! XX_COMMAND XX_OPT small.c -c -w > out.txt 2>&1 &&\
grep XX_STRING out.txt
