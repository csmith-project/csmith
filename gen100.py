import os

for seed in range(100):
    print("Generating with seed", seed) 
    os.system("src/csmith -s seed > " + str(seed)+ ".c")

