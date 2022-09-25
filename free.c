#include <creed.h>

void crFreeHashMap(struct CrHashMap *h) {
	free(h->cells);
}

void crFreeSym(struct CrSym *s) {
	free(s->d.p);
}

void crFreeGroup(struct CrGroup *g) {
	crFreeTok(g->tok);
	free(g->tok);
	if (g->next) {
		crFreeGroup(g->next);
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
