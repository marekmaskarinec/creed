#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "", "; \"a\" a\n\"b\" a"))
		return 1;

	CrSlice(wchar_t) exp = crUTF8ToSlice("b", 1);

	assert(exp.s == s.buf.s);
	assert(memcmp(exp.p, s.buf.p, exp.s * sizeof(wchar_t)) == 0);

	free(exp.p);
	crFreeState(&s);

	return 0;
}
