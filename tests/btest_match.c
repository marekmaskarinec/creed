#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "aabbccdd", "%end \"bc\" %/ \"cb\" s"))
		return 1;

	CrSlice(wchar_t) exp = crUTF8ToSlice("aabcbcdd", 8);

	assert(exp.s == s.buf.s);
	assert(memcmp(s.buf.p, exp.p, exp.s * sizeof(wchar_t)) == 0);

	free(exp.p);
	crFreeState(&s);

	return 0;
}
