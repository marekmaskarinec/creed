#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 32
#include <pcre2.h>

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
	wchar_t *b = malloc((s.s + 1) * sizeof(wchar_t));
	b[s.s] = 0;

	memcpy(b, s.p, s.s * sizeof(wchar_t));

	return (CrSlice(wchar_t)){
		.p = b,
		.s = s.s
	};
}

char *crReadAll(const char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "Could not open file %s %d\n", path, strlen(path));
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	size_t s = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = malloc((s + 1) * sizeof(char));
	buf[s] = 0;
	fread(buf, sizeof(char), s, f);	

	return buf;
}

struct CrErr crRegexMatch(
	CrSlice(wchar_t) *m,
	CrSlice(wchar_t) buf,
	CrSlice(wchar_t) pat
) {
	m->p = 0;
	m->s = 0;
	int err;
	size_t errOff;
	pcre2_code *code = pcre2_compile(
		(PCRE2_SPTR)pat.p, pat.s, PCRE2_UTF, &err, &errOff, NULL);

	if (err < 0) {
		PCRE2_UCHAR errBuf[BUFSIZ];
		pcre2_get_error_message(err, (PCRE2_UCHAR *)errBuf, BUFSIZ);
		fprintf(stderr, "regex compile error: at %zu: %ls (%d)\n",
			errOff, (wchar_t *)errBuf, err);
		return (struct CrErr){ .kind = CrErrRegexCompile };
	}

	pcre2_match_data *data;
	data = pcre2_match_data_create_from_pattern(code, NULL);
	int rc = pcre2_match(code, (PCRE2_SPTR)buf.p, buf.s, 0, 0, data, NULL);

	PCRE2_SIZE *offsets = pcre2_get_ovector_pointer(data);
	pcre2_get_ovector_count(data);

	if (rc < 0)
		goto end;

	m->p = buf.p + offsets[0];
	m->s = offsets[1] - offsets[0];

end:
	pcre2_code_free(code);
	pcre2_match_data_free(data);

	return (struct CrErr){0};
}
