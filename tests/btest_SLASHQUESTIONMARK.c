#include <assert.h>
#include <string.h>

#include <creed.h>

int main() {
	struct CrState s;
	if (crRunStr(&s, "aa bb cc", "%all \"bb\" %/ %end \"aa\" /? \"cc\" /?"))
		return 1;

	assert(s.stackBase[0].kind == CrValNum);
	assert(s.stackBase[0].num == 0);
	assert(s.stackBase[1].kind == CrValNum);
	assert(s.stackBase[1].num == 1);

	crFreeState(&s);

	return 0;
}
