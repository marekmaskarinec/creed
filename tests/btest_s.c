#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "aaaacc", "2 amp 2 aml \"bb\" s"))
		return 1;

	CrSlice(wchar_t) exp = crUTF8ToSlice("aabb", 4);

	assert(memcmp(s.buf.p, exp.p, exp.s * sizeof(wchar_t)) == 0);

	free(exp.p);
	crFreeState(&s);

	return 0;
}
