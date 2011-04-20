echo > delta.log

date >> delta.log
${CSMITH_PATH}/utah/scripts/reduce.sh >> delta.log
set -v
grep -c here1 delta.log >> delta.log
grep -c here2 delta.log >> delta.log
grep -c here3 delta.log >> delta.log
grep -c here4 delta.log >> delta.log
grep -c here5 delta.log >> delta.log
date >> delta.log
