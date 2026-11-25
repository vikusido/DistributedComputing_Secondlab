#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    double x, y;
} point;

/* Точка a левее точки b (при равных x — ниже). */
int less(point a, point b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
}

int compare(const void* a, const void* b) {
    point p = *(const point*)a;
    point q = *(const point*)b;
    if (less(p, q)) return -1;
    if (less(q, p)) return 1;
    return 0;
}

/* > 0, если c лежит слева от прямой a->b, < 0 — справа, 0 — на прямой. */
double cross(point a, point b, point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

int next(int i, int n) { return (i + 1) % n; }
int prev(int i, int n) { return (i + n - 1) % n; }

/*
 * Слияние оболочек L = h[0..nl) и R = h[mid..mid+nr).
 * Обе оболочки хранятся против часовой стрелки, начиная с самой левой
 * вершины. Результат записывается в h[0..), возвращается число вершин.
 */
int merge_hulls(point* h, int nl, int mid, int nr) {
    point* L = h;
    point* R = h + mid;

    /* Начинаем с самой правой вершины L и самой левой вершины R.
     * Самую левую вершину R искать не нужно: все оболочки хранятся
     * начиная с самой левой вершины, поэтому это всегда R[0]
     * (так их строят базовые случаи build_hull, и так же в конце
     * merge_hulls делает циклический сдвиг).
     * Самая правая вершина L может стоять где угодно в середине
     * массива, поэтому её ищем перебором, как максимум. */
    int l_start = 0;
    for (int i = 1; i < nl; ++i)
        if (less(L[l_start], L[i]))
            l_start = i;

    /* Верхняя опорная прямая: соседи l и r должны лежать ниже прямой l-r.
     * Если сосед выше (или на прямой, но дальше) — переходим к нему. */
    int l = l_start, r = 0, changed = 1;
    while (changed) {
        changed = 0;
        while (nl > 1) {
            int k = next(l, nl);
            double c = cross(L[l], R[r], L[k]);
            if (c > 0 || (c == 0 && less(L[k], L[l]))) { l = k; changed = 1; }
            else break;
        }
        while (nr > 1) {
            int k = prev(r, nr);
            double c = cross(L[l], R[r], R[k]);
            if (c > 0 || (c == 0 && less(R[r], R[k]))) { r = k; changed = 1; }
            else break;
        }
    }
    int l_top = l, r_top = r;

    /* Нижняя опорная прямая: соседи l и r должны лежать выше прямой l-r. */
    l = l_start; r = 0; changed = 1;
    while (changed) {
        changed = 0;
        while (nl > 1) {
            int k = prev(l, nl);
            double c = cross(L[l], R[r], L[k]);
            if (c < 0 || (c == 0 && less(L[k], L[l]))) { l = k; changed = 1; }
            else break;
        }
        while (nr > 1) {
            int k = next(r, nr);
            double c = cross(L[l], R[r], R[k]);
            if (c < 0 || (c == 0 && less(R[r], R[k]))) { r = k; changed = 1; }
            else break;
        }
    }
    int l_bottom = l, r_bottom = r;

    /* Новая оболочка: вершины L от l_top до l_bottom,
     * затем вершины R от r_bottom до r_top. */
    point* tmp = malloc((nl + nr) * sizeof(point));
    int n = 0;
    for (int i = l_top; ; i = next(i, nl)) {
        tmp[n++] = L[i];
        if (i == l_bottom) break;
    }
    for (int i = r_bottom; ; i = next(i, nr)) {
        tmp[n++] = R[i];
        if (i == r_top) break;
    }

    /* Сдвигаем так, чтобы первой снова была самая левая вершина. */
    int first = 0;
    for (int i = 1; i < n; ++i)
        if (less(tmp[i], tmp[first]))
            first = i;
    for (int i = 0; i < n; ++i)
        h[i] = tmp[(first + i) % n];
    free(tmp);
    return n;
}

/*
 * Оболочка отсортированных точек p[0..n) записывается в h[0..n).
 * Левая и правая половины обрабатываются параллельно в разных тасках:
 * они пишут в непересекающиеся части массива h, поэтому гонки нет.
 */
int build_hull(point* p, int n, point* h) {
    if (n == 1) {
        h[0] = p[0];
        return 1;
    }
    if (n == 2) {
        h[0] = p[0];
        h[1] = p[1];
        return 2;
    }
    if (n == 3) {
        double c = cross(p[0], p[1], p[2]);
        h[0] = p[0];
        if (c > 0) { h[1] = p[1]; h[2] = p[2]; return 3; }
        if (c < 0) { h[1] = p[2]; h[2] = p[1]; return 3; }
        h[1] = p[2];  /* три точки на одной прямой */
        return 2;
    }

    int mid = n / 2;  /* разделяющая прямая x = x_median */
    int nl = 0, nr = 0;
#pragma omp task shared(nl) if(n > 10000)
    nl = build_hull(p, mid, h);
#pragma omp task shared(nr) if(n > 10000)
    nr = build_hull(p + mid, n - mid, h + mid);
#pragma omp taskwait
    return merge_hulls(h, nl, mid, nr);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s thread_count < points.txt\n", argv[0]);
        return 1;
    }
    int thread_count = strtol(argv[1], NULL, 10);

    int n;
    if (scanf("%d", &n) != 1 || n <= 0)
        return 1;
    point* p = malloc(n * sizeof(point));
    point* h = malloc(n * sizeof(point));
    for (int i = 0; i < n; ++i)
        if (scanf("%lf %lf", &p[i].x, &p[i].y) != 2)
            return 1;

    double sort_start = omp_get_wtime();
    qsort(p, n, sizeof(point), compare);
    double sort_end = omp_get_wtime();

    int count = 0;
    double start = omp_get_wtime();
#pragma omp parallel num_threads(thread_count)
#pragma omp single
    count = build_hull(p, n, h);
    double end = omp_get_wtime();

    printf("%d\n", count);
    for (int i = 0; i < count; ++i)
        printf("%.17g %.17g\n", h[i].x, h[i].y);
    fprintf(stderr, "%lf %lf\n", sort_end - sort_start, end - start);

    free(p);
    free(h);
    return 0;
}
