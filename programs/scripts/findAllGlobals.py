import sys
import re

globPat = re.compile('__FLOAT (g.*?) = ')
def findDef(line):
  match = re.search(globPat, line)
  if match:
    print(match.group(1))

  
for line in open(sys.argv[1], 'r'):
  findDef(line)
  
