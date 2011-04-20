# optionally preprocess
# save sequential backup versions instead of hardcoding these

cp small.c small-1.c
godelta
cp small.c small-2.c
indent small.c
custom_delta.pl ./test1.sh ./small.c --all
cp small.c small-3.c
godelta
cp small.c small-4.c
indent small.c
cp small.c small-5.c
./test1.sh
cat small.c
wc small.c
