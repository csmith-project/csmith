import math
import sys
import re

def countLines(fname):
  res = 0
  with open(fname) as f:
    for line in f.readlines():
      res+=1
  return res

valuePattern = re.compile('^(.*?)= (.+)$')
def extractValue(line):
  match = re.search(valuePattern, line)
  if match:
    name = match.group(1)
    value = float.fromhex(match.group(2))
    return (name, value)
  else:
    return ("",0)

intervalPattern = re.compile('^(.*?)= \[(.*?), (.*?)\]')
def extractInterval(line):
  match = re.search(intervalPattern, line)
  if match:
    name = match.group(1)
    lower = float.fromhex(match.group(2))
    upper = float.fromhex(match.group(3))
    return (name, lower, upper)
  else: 
    return ("",0,0)

def isInInterval(value, lower, upper):
  return lower<=value and value<=upper

#arg[1] = values, arg[2] = intervals
result = 0

if(countLines(str(sys.argv[1])) != countLines(str(sys.argv[2]))):
  print("different numbers of lines")
  sys.exit(1)

intervals = {}
with open(str(sys.argv[2]), 'r') as f:
  for line in f.readlines():
    name, lower, upper = extractInterval(line)
    intervals[name] = (lower, upper)

with open(str(sys.argv[1]), 'r') as f:
  for line in f.readlines():
    name, value = extractValue(line)
    lower, upper = intervals[name]
    if math.isnan(value):
      if math.isfinite(lower) and math.isfinite(upper):
        print(name, value, lower, upper)
        result = 1
      continue
    if not isInInterval(value, lower, upper):
      print(name, value, lower, upper)
      result = 1

sys.exit(result)
