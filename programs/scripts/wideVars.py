import math
import sys
import re

intervalPattern = re.compile('= \[(.*?), (.*?)\]')
def extractInterval(line):
  match = re.search(intervalPattern, line)
  if match:
    lower = float.fromhex(match.group(1))
    upper = float.fromhex(match.group(2))
    return (lower, upper)
  else: 
    return (0,0)

def isInInterval(value, lower, upper):
  return lower<=value and value<=upper

f = open(str(sys.argv[1]), 'r')

for line in f.readlines():
  lower, upper = extractInterval(line)
  if lower < upper:
    print(line, end="")
print()
f.close()
