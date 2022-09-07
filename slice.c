#include <stdlib.h>

#include <creed.h>

void crSliceReserve(void **p, size_t *s, size_t membsiz, size_t *c, size_t r) {
	if ((*s) + r > *c) {
		void *pp = realloc(*p, 2 * (*s) * membsiz);
		if (pp == NULL) {
			crFatal("could not allocate %d bytes", 2 * (*s) * membsiz);
			exit(1);
		}

		*p = pp;
		*c *= 2;
	}
}
