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

for line1, line2 in zip(f1.readlines(), f2.readlines()):
  value = extractValue(line1)
  lower, upper = extractInterval(line2)
  if isInInterval(value, lower, upper):
    sys.exit(0)
  else:
    sys.exit(1)

f1.close()
f2.close()
