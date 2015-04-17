#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#define NUM_PT 32

const int stroke_infinity = 32768;

typedef struct  {
	int n;
	int thetas[32];
} stroke_t;

int stroke_compare(const stroke_t *a, const stroke_t *b) {
	inline int add_cost(int t1, int t2)
	{
		int ret = abs(t1-t2);
		// circle
		ret = ret < 8? ret: 16-ret;
		// dead-zone
		ret = ret < 2? 0: ret;
		return ret;
	}
	int m = a->n;
	int n = b->n;

	// DIST(y, x): the difference merit of a[0..x] and b[0..y]
#define DIST(y, x) (dist[(y)*NUM_PT+(x)])
	int dist[NUM_PT*NUM_PT];

	for (int y = 0; y < n; ++y)
		for (int x = 0; x < m; ++x)
			DIST(y, x) = stroke_infinity;
	DIST(0, 0) = add_cost(a->thetas[0], b->thetas[0]);

	for (int y = 0; y < n; ++y) {
		for (int x = 0; x < m; ++x) {
			if (DIST(y, x) >= stroke_infinity)
				continue;

			inline void step(int dy, int dx) {
				int x2 = x + dx;
				int y2 = y + dy;
				int ac = add_cost(a->thetas[x2], b->thetas[y2]);
				int new_dist = DIST(y, x) + ac;

				if (new_dist >= DIST(y2, x2))
					return;

				DIST(y2, x2) = new_dist;
			}
			if (x != m-1) {
				step(0, 1);
			}
			if (y != n-1) {
				step(1, 0);
			}
			if (x != m-1 && y != n-1) {
				step(1, 1);
			}
		}
	}
	return DIST(n-1, m-1);
}

void parse(const char filename[], stroke_t *s)
{
	FILE *fp = fopen(filename, "r");
	assert(fp);
	int x, y, theta, i;
	for (i = 0; i < NUM_PT; ++i) {
		if (fscanf(fp, "%d %d %d \n", &x, &y, &theta) == EOF) {
			break;
		}
		s->thetas[i] = theta;
	}
	s->n = i;
	fclose(fp);
}

int main(int argc, const char *argv[])
{
	assert(argc == 3);
	stroke_t a, b;
	parse(argv[1], &a);
	parse(argv[2], &b);
	printf("[%s] and [%s]: %d\n", argv[1], argv[2], stroke_compare(&a, &b));
	return 0;
}
