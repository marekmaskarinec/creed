#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "line 1\nline 2\nline 3\n", "mall \"2\" mrm mcl \"\" s"))
		return 1;

	CrSlice(wchar_t) exp = crUTF8ToSlice("line 1\nline 3\n", 14);

	assert(s.buf.s == exp.s);
	assert(memcmp(s.buf.p, exp.p, exp.s * sizeof(wchar_t)) == 0);

	free(exp.p);
	crFreeState(&s);
}
