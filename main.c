#include <locale.h>
#include <string.h>

#include <creed.h>

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");

	char *buf = "0 3 % 2 %% { @%. dump \"c+\" %/ \"C\" s } awas";

	struct CrLex lex = {};
	lex.buf = buf;
	lex.bufsiz = strlen(buf);

	/*do {
		crLexNext(&lex);
		printf("tok: %d %.*s\n", lex.tok.raw.s, lex.tok.raw.s, lex.tok.raw.p);
	} while (lex.tok.kind != CrTokEOF);*/

	struct CrGroup g;
	struct CrErr e = crParse(&lex, &g);
	if (e.kind) {
		crErrPrint(stderr, e);
		fprintf(stderr, "\n");
	}
	crGroupPrint(stderr, g);

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
	return 0;
}
