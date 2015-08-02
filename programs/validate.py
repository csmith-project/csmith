import math
import sys
import re

valuePattern = re.compile('= (.+)$')
def extractValue(line):
  match = re.search(valuePattern, line)
  if match:
    return float.fromhex(match.group(1))
  else:
    return "ERROR"

intervalPattern = re.compile('= \[(.*?), (.*?)\]')
def extractInterval(line):
  match = re.search(intervalPattern, line)
  if match:
    lower = float.fromhex(match.group(1))
    upper = float.fromhex(match.group(2))
    return (lower, upper)
  else: 
    return "ERROR"

def isInInterval(value, lower, upper):
  return lower<=value and value<=upper

#f1 - values, f2 - ranges
f1 = open(str(sys.argv[1]), 'r')
f2 = open(str(sys.argv[2]), 'r')

wide = 0
total = 0
result = 0
for line1, line2 in zip(f1.readlines(), f2.readlines()):
  total+=1
  value = extractValue(line1)
  lower, upper = extractInterval(line2)
  if math.isnan(value):
    continue
  if lower!=upper:
    wide+=1
  if not isInInterval(value, lower, upper):
    print(line1)
    print(line2)
    result = 1

print(total, wide)
f1.close()
f2.close()
sys.exit(result)
