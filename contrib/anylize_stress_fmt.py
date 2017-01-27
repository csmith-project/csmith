import pandas
import sys 
# Load the data into a DataFrame
#data = pandas.read_csv('times.csv')
data = pandas.read_csv(sys.stdin)
print data.describe()
  
