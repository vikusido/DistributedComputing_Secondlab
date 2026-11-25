#!/bin/bash
# Замер времени работы hull на разном количестве точек и потоков.
# Запуск: ./bench.sh > results.txt
# Вывод: точки, потоки, время сортировки, время параллельного построения
# оболочки (секунды). Ускорение считаем по последней колонке.
#
# Для каждого запуска в папку pictures/ сохраняются ответ программы
# (hull_N_T.txt) и картинка с оболочкой (hull_N_T.png).
# Ответ на каждом числе потоков сверяется с ответом на 1 потоке.

sizes=(100000 1000000 10000000)
threads=(1 2 4 8)

mkdir -p pictures

for n in "${sizes[@]}"
do
    python3 gen.py $n > points_$n.txt
    for t in "${threads[@]}"
    do
        echo -n "$n $t "
        ./hull $t < points_$n.txt 2>&1 > pictures/hull_${n}_${t}.txt
        python3 visualize.py points_$n.txt pictures/hull_${n}_${t}.txt pictures/hull_${n}_${t}.png
        if ! cmp -s pictures/hull_${n}_1.txt pictures/hull_${n}_${t}.txt
        then
            echo "ОШИБКА: N=$n, $t потоков — ответ отличается от 1 потока" >&2
        fi
    done
done
