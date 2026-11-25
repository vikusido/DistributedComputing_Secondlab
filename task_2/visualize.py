# Визуализация: python3 visualize.py points.txt hull.txt [picture.png]
# Серые точки — исходные, красный многоугольник — выпуклая оболочка.
# Если указан третий аргумент, картинка сохраняется в этот файл без
# открытия окна (так её вызывает bench.sh), иначе — в hull.png и окно.
import random
import sys

import matplotlib
if len(sys.argv) > 3:
    matplotlib.use("Agg")   # рисуем сразу в файл, без окна
import matplotlib.pyplot as plt


def read(path):
    numbers = open(path).read().split()
    n = int(numbers[0])
    xs = [float(numbers[1 + 2 * i]) for i in range(n)]
    ys = [float(numbers[2 + 2 * i]) for i in range(n)]
    return xs, ys


px, py = read(sys.argv[1])
hx, hy = read(sys.argv[2])
n = len(px)

# Миллионы точек сливаются в пятно и рисуются долго,
# поэтому показываем случайные 20000 из них.
if n > 20000:
    idx = random.sample(range(n), 20000)
    px = [px[i] for i in idx]
    py = [py[i] for i in idx]

plt.scatter(px, py, s=3 if n <= 1000 else 0.5, color="gray")
plt.plot(hx + hx[:1], hy + hy[:1], color="red")   # замыкаем многоугольник
plt.scatter(hx, hy, s=10, color="red")
plt.gca().set_aspect("equal")
plt.title(f"N = {n}, вершин оболочки = {len(hx)}")

if len(sys.argv) > 3:
    plt.savefig(sys.argv[3])
else:
    plt.savefig("hull.png")
    plt.show()
