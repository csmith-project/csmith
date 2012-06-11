set -e
while [ true ]
do
  echo .
  csmith > test.c;
  clang -I${CSMITH_PATH}/runtime -O3 -w test.c -o /dev/null;
  gcc -I${CSMITH_PATH}/runtime -O3 -w test.c -o /dev/null;
done
