import math
import sys
import re

intervalPattern = re.compile('^(.*?)= \[(.*?), (.*?)\]')
def extractInterval(line):
  match = re.search(intervalPattern, line)
  if match:
    name = match.group(1)
    lower = float.fromhex(match.group(2))
    upper = float.fromhex(match.group(3))
    return (name, lower, upper)
  else: 
    return (0,0)

def isInInterval(value, lower, upper):
  return lower<=value and value<=upper

f = open(str(sys.argv[1]), 'r')

wide = {}

for line in f.readlines():
  name, lower, upper = extractInterval(line)
  if lower < upper:
    wide[name] = (line)
for key in wide:
  print(wide[key], end="")
f.close()
