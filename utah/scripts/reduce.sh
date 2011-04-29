# optionally preprocess
# save sequential backup versions instead of hardcoding these

DIR=${CSMITH_PATH}/utah/scripts

cp small.c small-1.c
perl -pi.bak -e 's/^\s*\n//g' small.c
${DIR}/godelta
cp small.c small-2.c
indent small.c
${DIR}/custom_delta.pl ./test1.sh ./small.c --all
cp small.c small-3.c
perl -pi.bak -e 's/^\s*\n//g' small.c
${DIR}/godelta
cp small.c small-4.c
indent small.c
cp small.c small-5.c
./test1.sh
cat small.c
wc small.c
