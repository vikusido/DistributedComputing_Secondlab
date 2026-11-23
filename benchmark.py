#!/usr/bin/env python3
"""Запускает ./pi для разных N и числа потоков, сохраняет CSV и строит графики."""

import csv
import statistics
import subprocess

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

EXE = "./pi"
POINTS = [100_000, 1_000_000, 10_000_000, 100_000_000]
THREADS = [1, 2, 4, 6, 8, 12]   # первым должен идти 1 - от него считаем ускорение
REPEATS = 5                     # сколько раз повторяем каждый запуск


def run(n, threads):
    """Запускает pi и разбирает строку: threads=4 n=1000 pi=3.14 error=0.001 time=0.01"""
    out = subprocess.run([EXE, str(n), str(threads)],
                         check=True, capture_output=True, text=True).stdout
    return dict(part.split("=") for part in out.split())


# замеры 
rows = []
for n in POINTS:
    print(f"N = {n}")
    base_time = None                      # время на одном потоке
    for threads in THREADS:
        times = []
        for _ in range(REPEATS):
            result = run(n, threads)
            if int(result["threads"]) != threads:
                print(f"  внимание: запрошено {threads} потоков, создано {result['threads']}")
            times.append(float(result["time"]))

        time = statistics.median(times)   # медиана меньше зависит от случайных выбросов
        if threads == 1:
            base_time = time
        speedup = base_time / time
        rows.append({
            "points": n,
            "threads": threads,
            "pi": float(result["pi"]),
            "error": float(result["error"]),
            "time": time,
            "speedup": speedup,
            "efficiency": speedup / threads,
        })
        print(f"  потоков {threads}: время {time:.4f} с, ускорение {speedup:.2f}")

# таблица
with open("results.csv", "w", newline="", encoding="utf-8") as file:
    writer = csv.DictWriter(file, fieldnames=rows[0].keys())
    writer.writeheader()
    writer.writerows(rows)

# графики
fig, axes = plt.subplots(2, 2, figsize=(12, 8))
ax_time, ax_speed, ax_eff, ax_err = axes.flatten()

for n in POINTS:
    part = [r for r in rows if r["points"] == n]
    ax_time.plot(THREADS, [r["time"] for r in part], "o-", label=f"N = {n}")
    ax_speed.plot(THREADS, [r["speedup"] for r in part], "o-", label=f"N = {n}")
    ax_eff.plot(THREADS, [r["efficiency"] for r in part], "o-", label=f"N = {n}")

ax_time.set_title("Время выполнения")
ax_time.set_ylabel("Время, с")
ax_time.set_yscale("log")

ax_speed.plot(THREADS, THREADS, "--", color="gray", label="идеальное")
ax_speed.set_title("Ускорение")
ax_speed.set_ylabel("T1 / Tp")

ax_eff.set_title("Эффективность")
ax_eff.set_ylabel("Ускорение / число потоков")

for ax in (ax_time, ax_speed, ax_eff):
    ax.set_xlabel("Число потоков")
    ax.set_xticks(THREADS)
    ax.legend()
    ax.grid(True)

# ошибка берётся из запусков с одним потоком
one = [r for r in rows if r["threads"] == 1]
ax_err.loglog(POINTS, [r["error"] for r in one], "o-")
ax_err.set_title("Ошибка в зависимости от N")
ax_err.set_xlabel("Число точек N")
ax_err.set_ylabel("|π - π_оценка|")
ax_err.grid(True)

fig.tight_layout()
fig.savefig("plots.png", dpi=150)
print("Готово: results.csv, plots.png")