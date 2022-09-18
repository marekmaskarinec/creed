#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <creed.h>

#define MAX_NEST 256

static
struct CrErr parse(struct CrLex *lex, struct CrGroup *out, int level) {
	struct CrGroup *group = out;
	struct CrGroup *prev = NULL;

	crLexNext(lex);
	while (lex->tok.kind != CrTokEOF) {
		struct CrTok *tok = NULL;
		if (lex->tok.kind != CrTokGroupEnd) {
			group->tok = malloc(sizeof(struct CrTok));
			*group->tok = lex->tok;
			tok = group->tok;
      
			prev = group;
			group = group->next = malloc(sizeof(struct CrGroup));
		}

		switch (lex->tok.kind) {
		case CrTokGroupBegin: 
			CHECKOUT(parse(lex, &tok->group, level + 1));
			break;

		case CrTokGroupEnd:
			if (prev && prev->next) {
				free(prev->next);
				prev->next = NULL;
			}

			ASSERT(level == 0, CrErrUnexpectedToken, *tok);
			return (struct CrErr){ .kind = CrErrNull };
		
		case CrTokString: {
			tok->str = crUTF8ToSlice(tok->raw.p, (size_t)tok->raw.s);

			break;

		} case CrTokNumber: {
			int n = 0;

			char *s = tok->raw.p;
			if (tok->raw.p[0] == '-')
				++s;

			int m = pow(10, tok->raw.p + tok->raw.s - s - 1);
			for (; s < tok->raw.p + tok->raw.s; ++s) {
				n += (*s - '0') * m;
				m /= 10;
			}

			tok->num = tok->raw.p[0] == '-' ? -n : n;
			break;

		} case CrTokSymbol:
			tok->sym.d = tok->raw;
			tok->sym.h = crHash(tok->sym.d);

			break;

		default:
			break;
		}

		crLexNext(lex);
	}

	if (prev && prev->next) {
		free(prev->next);
		prev->next = NULL;
	}

	ASSERT(level > 0, CrErrUnterminatedGroup, lex->tok);
	return (struct CrErr){ .kind = CrErrNull };
}

struct CrErr crParse(struct CrLex *lex, struct CrGroup *out) {
	return parse(lex, out, 0);
}

struct CrErr crParseStr(char *str, struct CrGroup *out) {
	struct CrLex lex = {0};
	lex.buf = str;
	lex.bufsiz = strlen(str);

	CHECKOUT(crParse(&lex, out));

	return (struct CrErr){0};
}

struct CrErr crParseFile(const char *path, struct CrGroup *out) {
	char *buf = crReadAll(path);

	CHECKOUT(crParseStr(buf, out));
	free(buf);

	return (struct CrErr){0};
}
