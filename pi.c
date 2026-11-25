#include <errno.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Переводит строку в положительное целое число
// Возвращает 1, если вся строка - корректное число > 0, иначе 0
static int read_positive(const char *text, long long *value)
{
    char *end;
    errno = 0;
    long long number = strtoll(text, &end, 10);

    if (end == text || *end != '\0')
    { // пусто или после числа есть мусор
        return 0;
    }
    if (errno == ERANGE || number <= 0)
    { // слишком большое число или не положительное
        return 0;
    }
    *value = number;
    return 1;
}

int main(int argc, char **argv)
{
    long long n;            // число точек
    long long thread_count; // запрошенное число потоков

    if (argc != 3 ||
        !read_positive(argv[1], &n) ||
        !read_positive(argv[2], &thread_count) ||
        thread_count > INT_MAX)
    {
        fprintf(stderr, "Использование: %s N потоки\n"
                        "N и потоки - целые числа больше 0.\n",
                argv[0]);
        return 1;
    }

    long long hits = 0;     // попаданий в круг
    int actual_threads = 0; // сколько потоков реально создано

    double start = omp_get_wtime();

#pragma omp parallel num_threads((int)thread_count) reduction(+ : hits)
    {
        // Реальное число потоков может быть меньше запрошенного
        // (например, из-за OMP_THREAD_LIMIT), поэтому спрашиваем его внутри параллельной области. Записывает только один поток

#pragma omp single
        actual_threads = omp_get_num_threads();

        // У каждого потока свой seed, чтобы не было гонки данных
        unsigned int seed = 12345 + omp_get_thread_num();

#pragma omp for
        for (long long i = 0; i < n; ++i)
        {
            double x = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // генерирует случайную координату x
            double y = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // генерирует случайную координату y
            if (x * x + y * y <= 1.0)
            {
                hits++;
            }
        }
    }

    double end = omp_get_wtime();
    double pi = 4.0 * hits / n;

    printf("threads=%d n=%lld pi=%.8f error=%.8f time=%.6f\n",
           actual_threads, n, pi, fabs(pi - 3.14159265358979323846), end - start);
    return 0;
}