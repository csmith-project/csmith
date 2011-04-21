#!/bin/bash

# todo: these come from environment variables

CC1='current-gcc -Ofast -funroll-loops -fomit-frame-pointer -fwhole-program'
CRASHSTR='eliminate_redundant_comparison'

rm -f out*.txt

if 
  echo here1 &&\
  current-gcc -Wall -O small.c  >outa.txt 2>&1 &&\
  ! grep uninitialized outa.txt &&\
  ! grep 'control reaches end' outa.txt &&\
  ! grep 'incompatible implicit' outa.txt &&\
  clang -pedantic -Wall -O0 -c small.c  >out.txt 2>&1 &&\
  ! grep 'eliding middle term' out.txt &&\
  ! grep 'end of non-void function' out.txt &&\
  ! grep 'invalid in C99' out.txt &&\
  ! grep 'should return a value' out.txt &&\
  ! grep uninitialized out.txt &&\
  ! grep 'incompatible pointer' out.txt &&\
  ! grep 'type specifier missing' out.txt &&\
  echo here2 &&\
  ! $CC1 small.c -o small1 > cc_out1.txt 2>&1 &&\
  grep $CRASHSTR cc_out1.txt &&\
  echo here3 &&\
  ~/c-semantics-bundle-0.1/c-semantics/dist/kcc -s small.c >/dev/null 2>&1 &&\
  echo here4 
then
  if 
    ./a.out 
  then
    echo here5
    exit 0
  else
    TMPF="undefined_$$.c"
    echo copying C file to $TMPF
    cp small.c $TMPF
    exit 1
  fi
else
  exit 1
fi
