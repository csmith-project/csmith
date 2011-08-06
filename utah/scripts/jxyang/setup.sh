#!/bin/sh

sudo -s
cd /z
tar xvfz /proj/randtest/compilers-64bit-ubuntu.tar.gz   
rm -rf llvm-gcc-25
cp -r /proj/randtest/csmith .
export CSMITH_HOME=/z/csmith  
export COMPILER_HOME=/z 
export PATH=$PATH:/z/csmith/scripts
cd /z/csmith/src; g++ *.cpp -DPACKAGE_STRING -o csmith
cd /z/csmith/scripts/
chmod 755 *
# change the following line to the Csmith option you want to disable/enable
# also change the number of cores if necessary
# ./parallel_test.pl 8 compilers.txt --no-math64