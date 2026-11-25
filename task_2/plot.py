# Графики времени, ускорения и эффективности: python3 plot.py results.txt
# results.txt — вывод bench.sh: точки, потоки, время сортировки, время
# параллельной части. Считаем по времени параллельной части (4-я колонка).
import sys
import matplotlib.pyplot as plt

# Читаем результаты: data[n] = список пар (потоки, время)
data = {}
for line in open(sys.argv[1]):
    n, t, sort_time, hull_time = line.split()
    data.setdefault(int(n), []).append((int(t), float(hull_time)))

fig, (ax_time, ax_speedup, ax_eff) = plt.subplots(1, 3, figsize=(15, 4.5))

for n, rows in data.items():
    threads = [t for t, _ in rows]
    times = [time for _, time in rows]
    t1 = times[0]                                          # время на 1 потоке
    speedup = [t1 / time for time in times]                # S = T1 / Tp
    efficiency = [s / p for s, p in zip(speedup, threads)]  # E = S / p

    ax_time.plot(threads, times, "o-", label=f"N = {n}")
    ax_speedup.plot(threads, speedup, "o-", label=f"N = {n}")
    ax_eff.plot(threads, efficiency, "o-", label=f"N = {n}")

all_threads = sorted({t for rows in data.values() for t, _ in rows})
ax_speedup.plot(all_threads, all_threads, "k--", label="идеальное")
ax_eff.axhline(1, color="k", linestyle="--", label="идеальная")

ax_time.set_title("Время работы")
ax_time.set_ylabel("секунды (логарифмическая шкала)")
ax_time.set_yscale("log")   # чтобы на одном графике были видны и малые, и большие N
ax_speedup.set_title("Ускорение S = T1 / Tp")
ax_eff.set_title("Эффективность E = S / p")
for ax in (ax_time, ax_speedup, ax_eff):
    ax.set_xlabel("число потоков p")
    ax.set_xticks(all_threads)
    ax.grid(True)
    ax.legend()

fig.tight_layout()
plt.savefig("graphs.png")
plt.show()
