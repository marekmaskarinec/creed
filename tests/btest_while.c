#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "", "10 { 1 sub dup } { \".\" a } while"))
		return 1;

	CrSlice(wchar_t) exp = crUTF8ToSlice(".........", 9);

	assert(s.buf.s == exp.s);
	assert(memcmp(s.buf.p, exp.p, exp.s * sizeof(wchar_t)) == 0);

	free(exp.p);
	crFreeState(&s);

	return 0;
}
