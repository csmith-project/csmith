# Usage: python stress_fmt.py | python anylize_stress_fmt.py
# Requires clang-format in your path
import os
import time

print "clang_fmt_llvm, clang_fmt_google, clang_raw, clang_llvm, clang_google"
for x in xrange(100):
	
	os.system("../src/csmith > test.c")
	t1 = time.time()
	os.system("clang-format -style=LLVM test.c > test_llvm.c")
	t2 = time.time()
	os.system("clang-format -style=Google test.c > test_google.c")
	t3 = time.time()	
	os.system("clang -I../runtime -O -w test.c -o /dev/null")
	t4 = time.time()
	os.system("clang -I../runtime -O -w test_llvm.c -o /dev/null")
	t5 = time.time()
	os.system("clang -I../runtime -O -w test_google.c -o /dev/null")
	t6 = time.time()
	
	print '{0},{1},{2},{3},{4}'.format(t2-t1, t3-t2, t4-t3, t5-t4, t6-t5)
