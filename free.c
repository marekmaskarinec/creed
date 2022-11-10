#include <creed.h>

void crFreeHashMap(struct CrHashMap *h) {
	if (h->freeFn)
		for (int i=0; i < h->size; ++i)
			if (h->cells[i].p)
				h->freeFn(h, &h->cells[i]);
	free(h->cells);
}

void crFreeSym(struct CrSym *s) {
	free(s->d.p);
}

void crFreeGroup(struct CrGroup *g) {
	for (int i=0; i < g->len; ++i) {
		crFreeTok(g->toks[i]);
		free(g->toks[i]);
	}

	free(g->toks);
}

void crFreeParGroup(struct CrParGroup *g) {
	crFreeTok(g->tok);
	free(g->tok);
	if (g->next) {
		crFreeParGroup(g->next);
		free(g->next);
	}
}

void crFreeTok(struct CrTok *t) {
	switch (t->kind) {
	case CrTokString:
		free(t->str.p);
		break;
	case CrTokGroupBegin:
		crFreeGroup(&t->group);
		break;
	case CrTokQuote:
	case CrTokSymbol:
		crFreeSym(&t->sym);
		break;
	default:
		break;
	}
}

void crFreeLex(struct CrLex *lex) {
	free(lex->buf);
}

void crFreeVal(struct CrVal *v) {
	switch (v->kind) {
	case CrValStr:
		free(v->str.p);
		break;
	case CrValGroup:
		crFreeGroup(&v->group);
		break;
	case CrValSym:
		crFreeSym(&v->sym);
		break;
	default:
		break;
	}
}

void crFreeState(struct CrState *s) {
	free(s->buf.p);

	crFreeHashMap(&s->builtins);
	crFreeHashMap(&s->syms);
}
