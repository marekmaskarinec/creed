#include <string.h>

#include <creed.h>

void *crDupMem(void *p, size_t s) {
	void *r = malloc(s);
	memcpy(r, p, s);
	return r;
}

#define DUP(p) crDupMem((p), sizeof(*(p)))

struct CrHashMap *crDupHashMap(struct CrHashMap *t, struct CrHashMap *h) {
	t->cells = DUP(h->cells);
	t->size = h->size;

	return t;
}

struct CrSym *crDupSym(struct CrSym *t, struct CrSym *s) {
	t->d.p = crDupMem(s->d.p, s->d.s * sizeof(char));
	t->d.s = s->d.s;
	return t;
}

struct CrGroup *crDupGroup(struct CrGroup *t, struct CrGroup *g) {
	*t = *g;

	crDupTok(t->tok, g->tok);
	if (g->next)
		t->next = crDupGroup(DUP(g->next), g->next);

	return t;
}

struct CrTok *crDupTok(struct CrTok *tgt, struct CrTok *t) {
	*tgt = *t;

	switch (t->kind) {
	case CrTokString:
		tgt->str.p = crDupMem(t->str.p, t->str.s * sizeof(wchar_t));
		break;
	case CrTokGroupBegin:
		crDupGroup(&tgt->group, &t->group);
		break;
	case CrTokSymbol:
		crDupSym(&tgt->sym, &t->sym);
		break;
	default:
		break;
	}

	return tgt;
}

struct CrLex *crDupLex(struct CrLex *t, struct CrLex *lex) {
	*t = *lex;

	t->buf = DUP(lex->buf);

	return t;
}

struct CrVal *crDupVal(struct CrVal *t, struct CrVal *v) {
	*t = *v;

	switch (v->kind) {
	case CrValStr:
		t->str.p = crDupMem(v->str.p, t->str.s * sizeof(wchar_t));
		break;
	case CrValGroup:
		crDupGroup(&t->group, &v->group);
		break;
	case CrValSym:
		crDupSym(&t->sym, &v->sym);
		break;
	default:
		break;
	}

	return t;
}
