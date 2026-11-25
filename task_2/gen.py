# Генерация случайных точек: python3 gen.py N > points.txt
import random
import sys

n = int(sys.argv[1])
print(n)
for _ in range(n):
    print(random.uniform(-1, 1), random.uniform(-1, 1))
