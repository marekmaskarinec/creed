#include <locale.h>
#include <string.h>

#include <creed.h>

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");

	char *buf = "0 \"date -Iseconds\" ! dump drop";
	/*do {
		crLexNext(&lex);
		printf("tok: %d %.*s\n", lex.tok.raw.s, lex.tok.raw.s, lex.tok.raw.p);
	} while (lex.tok.kind != CrTokEOF);*/

	struct CrGroup g;
	struct CrErr e = crParseStr(buf, &g);
	if (e.kind) {
		crErrPrint(stderr, e);
		fprintf(stderr, "\n");
	}

	//crGroupPrint(stderr, g);

	struct CrState s;
	crStateInit(&s);
	crStateSetBuf(&s, "aůbbccddeeff");

	fprintf(stderr, "input string(%zu): %ls\n", s.buf.s, s.buf.p);
	fprintf(stderr, "executing: %s\n", buf);

	e = crEval(&s, &g);
	if (e.kind) {
		crErrPrint(stderr, e);
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "buf: %.*ls\n", (int)s.buf.s + 1, s.buf.p);

	crFreeState(&s);
	crFreeGroup(&g);

	return 0;
}
