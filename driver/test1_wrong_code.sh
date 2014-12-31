#!/bin/bash
##
## Copyright (c) 2012, 2013, 2014 The University of Utah
## All rights reserved.
##
## This file is distributed under the University of Illinois Open Source
## License.  See the file COPYING for details.

###############################################################################

# command: XX_COMMAND
# string: XX_STRING
# crashfile: XX_CRASHFILE
# dir: XX_DIR
# compiler: XX_COMPILER
# bad opt: XX_OPT
# good_opt: XX_GOOD

rm -f out*.txt

# this should mirror the timeout in run_program 
PROG_TIMEOUT=5

ulimit -t 60
ulimit -v 8000000
ulimit -m 8000000

# ${CSMITH_HOME}/driver/check_unique.pl XX_CRASHFILE XX_DIR XX_OPT "XX_COMMAND"

if 
  clang -pedantic -Wall -O -fsanitize=undefined -fno-sanitize-recover small.c -o smallz >out_clang.txt 2>&1 &&\
  ! grep 'conversions than data arguments' out_clang.txt &&\
  ! grep 'incompatible redeclaration' out_clang.txt &&\
  ! grep 'ordered comparison between pointer' out_clang.txt &&\
  ! grep 'eliding middle term' out_clang.txt &&\
  ! grep 'end of non-void function' out_clang.txt &&\
  ! grep 'invalid in C99' out_clang.txt &&\
  ! grep 'specifies type' out_clang.txt &&\
  ! grep 'should return a value' out_clang.txt &&\
  ! grep 'uninitialized' out_clang.txt &&\
  ! grep 'incompatible pointer to' out_clang.txt &&\
  ! grep 'incompatible integer to' out_clang.txt &&\
  ! grep 'type specifier missing' out_clang.txt &&\
  RunSafely ${PROG_TIMEOUT} 1 /dev/null out_small1.txt ./smallz &&\
  gcc -Wall -Wextra -O1 small.c -o smallz >out_gcc.txt 2>&1 &&\
  ! grep 'uninitialized' out_gcc.txt &&\
  ! grep 'without a cast' out_gcc.txt &&\
  ! grep 'control reaches end' out_gcc.txt &&\
  ! grep 'return type defaults' out_gcc.txt &&\
  ! grep 'cast from pointer to integer' out_gcc.txt &&\
  ! grep 'useless type name in empty declaration' out_gcc.txt &&\
  ! grep 'no semicolon at end' out_gcc.txt &&\
  ! grep 'type defaults to' out_gcc.txt &&\
  ! grep 'too few arguments for format' out_gcc.txt &&\
  ! grep 'in format' out_gcc.txt &&\
  ! grep 'incompatible pointer' out_gcc.txt &&\
  ! grep 'ordered comparison of pointer with integer' out_gcc.txt &&\
  ! grep 'declaration does not declare anything' out_gcc.txt &&\
  ! grep 'expects type' out_gcc.txt &&\
  ! grep 'pointer from integer' out_gcc.txt &&\
  ! grep 'incompatible implicit' out_gcc.txt &&\
  ! grep 'excess elements in struct initializer' out_gcc.txt &&\
  ! grep 'comparison between pointer and integer' out_gcc.txt &&\
  XX_COMMAND XX_OPT small.c -o small1 > /dev/null 2>&1 &&\
  RunSafely ${PROG_TIMEOUT} 1 /dev/null out_small1.txt ./small1 &&\
  XX_COMMAND XX_GOOD small.c -o small2 > /dev/null 2>&1 &&\
  RunSafely ${PROG_TIMEOUT} 1 /dev/null out_small2.txt ./small2 &&\
  ! diff out_small1.txt out_small2.txt &&\
  # these options assume Frama-C Fluorine and a 64-bit machine
  frama-c -cpp-command 'gcc -C -Dvolatile= -E -I.' -val -no-val-show-progress -machdep x86_64 -obviously-terminates small.c > out_framac.txt 2>&1 &&\
  ! egrep -i '(user error|assert)' out_framac.txt >/dev/null 2>&1
then
  exit 0
else
  exit 1
fi
