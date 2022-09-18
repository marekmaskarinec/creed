#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

CrSlice(wchar_t) crStrDup(CrSlice(wchar_t) s) {
	wchar_t *b = malloc(s.s * sizeof(wchar_t));
	b[s.s] = 0;

	memcpy(b, s.p, s.s * sizeof(wchar_t));

	return (CrSlice(wchar_t)){
		.p = b,
		.s = s.s
	};
}

char *crReadAll(const char *path) {
	FILE *f = fopen(path, "r");

	fseek(f, SEEK_END, 0);
	size_t s = ftell(f);
	fseek(f, SEEK_SET, 0);

	char *buf = malloc((s + 1) * sizeof(char));
	fread(buf, sizeof(char), s, f);	

	return buf;
}
