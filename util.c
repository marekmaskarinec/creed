#include <stdarg.h>
#include <stdio.h>

#include <creed.h>

void crFatal(char *msg, ...) {
	fprintf(stderr, "creed: fatal: ");

	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);

	fprintf(stderr, "\n");
}

unsigned crHash(CrSlice(char) s) {
	unsigned h = 5381;

	for (int i=0; i < s.s; ++i)
		h = ((h << 5) + h) + s.p[i];

	return h;
}

char *crWsToMb(CrSlice(wchar_t) s) {
	// the worst case scenario
	char *buf = calloc(s.s, sizeof(char) * 4);
	wcstombs(buf, s.p, s.s * 4);
	return buf;
}
