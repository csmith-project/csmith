import sys
import re

globPat = re.compile('__FLOAT (g[^\[]*?) = ')
localPat = re.compile('__FLOAT (l[^\]]*?) = ')

def findDef(line, pat):
  match = re.search(pat, line)
  if match:
    print(match.group(1))

arrayPat = re.compile('__FLOAT (((g|l).*?)((\[[0-9]*\])+)) = ')
def findArray(line):
  match = re.search(arrayPat, line)
  if match:
    var = match.group(2)
    dims = match.group(4)
    dims = dims[1:(len(dims)-1)].split('][')
    dims = list(map(int, dims))
    for x in dimGen(dims):
      print(var+'['+ ']['.join(list(map(str, x))) + ']')
  
def dimGen(dims):
  if len(dims) == 0:
    yield []
    return
  for x in range(dims[0]):
    for y in dimGen(dims[1:]):
      yield [x]+y


for line in open(sys.argv[1], 'r'):
  findDef(line, globPat)
  findDef(line, localPat)
  #findArray(line)
