#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include <creed.h>

static
int stringp(int c) {
	return c != '"';
}

static
int symbolp(int c) {
	return !isspace(c) && isprint(c);
}

static
int numberp(int c) {
	return isdigit(c) || c == '-';
}

static
char peekc(struct CrLex *lex) {
	return lex->loc.idx >= lex->bufsiz ? 0 : lex->buf[lex->loc.idx];
}

static
char nextc(struct CrLex *lex) {
	char c = peekc(lex);
	if (c == '\n') {
		lex->loc.cno = 0;
		++lex->loc.lno;
	} else {
		++lex->loc.cno;
	}

	++lex->loc.idx;
	return c;
}

static
size_t eat(struct CrLex *lex, int (*p)(int c)) {
	size_t s = 0;

	while (p(peekc(lex))) {
		nextc(lex);
		++s;
	}

	return s;
}

struct CrErr crLexNext(struct CrLex *lex) {
	eat(lex, isspace);

	lex->tok.raw.p = &lex->buf[lex->loc.idx];
	lex->tok.loc = lex->loc;
	switch (peekc(lex)) {
	case 0:
		lex->tok.kind = CrTokEOF;
		break;
	case '{':
		lex->tok.raw.s = 1;
		lex->tok.kind = CrTokGroupBegin;
		nextc(lex);
		break;
	case '}':
		lex->tok.raw.s = 1;
		lex->tok.kind = CrTokGroupEnd;
		nextc(lex);
		break;
	case '"':
		++lex->tok.raw.p;
		nextc(lex);

		lex->tok.raw.s = eat(lex, stringp);
		lex->tok.kind = CrTokString;

		nextc(lex);
		break;
	case '\'':
		++lex->tok.raw.p;
		nextc(lex);

		lex->tok.raw.s = eat(lex, symbolp);
		lex->tok.kind = CrTokQuote;

		break;
	default:
		if (isdigit(peekc(lex)) || peekc(lex) == '-') {
			lex->tok.raw.s = eat(lex, numberp);
			lex->tok.kind = CrTokNumber;
		} else {
			lex->tok.raw.s = eat(lex, symbolp);
			lex->tok.kind = CrTokSymbol;
		}
	}

	return (struct CrErr){ .kind = CrErrNull, .tok = lex->tok };
}

