#include <regex.h>
#include <string.h>
#include <locale.h>

#include <creed.h>

static
void symsHmFreeFn(struct CrHashMap *self, struct CrHashMapCell *c) {
	crFreeGroup(c->p);
	free(c->p);
}

void crStateInit(struct CrState *state) {
	crHashMapInit(&state->builtins);
	crHashMapInit(&state->syms);
	state->syms.freeFn = symsHmFreeFn;

	state->stack = state->stackBase - 1;

	crAttachBuiltins(state);
}

void crStateSetBuf(struct CrState *state, char *s) {
	state->buf = crUTF8ToSlice(s, strlen(s));
	state->mark.p = state->buf.p;
	state->mark.s = 0;
}

void crStateAddBuiltin(struct CrState *state, char *name, struct CrErr (*fn)(struct CrState *)) {
	crHashMapSetStr(&state->builtins, name, fn);
}

struct CrErr crStatePushTok(struct CrState *state, struct CrTok *tok) {
	ASSERT(state->stack >= state->stackBase + STACK_SIZE, CrErrStackOverflow, *tok);

	++state->stack;

	switch (tok->kind) {
	case CrTokGroupBegin:
		state->stack->kind = CrValGroup;
		crDupGroup(&state->stack->group, &tok->group);
		break;

	case CrTokString:
		state->stack->kind = CrValStr;
		state->stack->str = crStrDup(tok->str);
		break;

	case CrTokNumber:
		state->stack->kind = CrValNum;
		state->stack->num = tok->num;
		break;

	case CrTokQuote:
		state->stack->kind = CrValSym;
		crDupSym(&state->stack->sym, &tok->sym);
		break;

	default:
		break;
	}

	return (struct CrErr){ .kind = CrErrNull };
}

struct CrErr crStatePush(struct CrState *state, struct CrVal val) {
	ASSERT(state->stack >= state->stackBase + STACK_SIZE, CrErrStackOverflow, *state->tok);
	++state->stack;
	*state->stack = val;

	return (struct CrErr){0};
}

void crStateSubst(struct CrState *state, CrSlice(wchar_t) mark, CrSlice(wchar_t) text) {
	const size_t s = state->buf.s + text.s - mark.s;
	wchar_t *p = malloc((s + 1) * sizeof(wchar_t));
	p[s] = 0;

	const wchar_t *bufEnd = state->buf.p + state->buf.s;
	const wchar_t *markEnd = mark.p + mark.s;
	const size_t markIdx = mark.p - state->buf.p;

	if (markIdx > 0)
		memcpy(p, state->buf.p, markIdx * sizeof(wchar_t));
	memcpy(p + markIdx, text.p, text.s * sizeof(wchar_t));
	if (markEnd < bufEnd)
		memcpy(
					 p + markIdx + text.s,
					 markEnd,
					 (bufEnd - markEnd) * sizeof(wchar_t));

	free(state->buf.p);
	state->buf.p = p;
	state->buf.s = s;

	state->mark.p = state->buf.p + markIdx;
	state->mark.s = text.s;
}

struct CrErr crStatePop(struct CrState *state, struct CrVal *out) {
	ASSERT(state->stack < state->stackBase, CrErrStackUnderflow, *state->tok);

	*out = *state->stack;
	--state->stack;

	return (struct CrErr){0};
}

struct CrErr crStatePopTyped(struct CrState *state, struct CrVal *out, enum CrValKind type) {
	CHECKOUT(crStatePop(state, out));
	ASSERT(out->kind != type, CrErrTypeError, *state->tok);

	return (struct CrErr){0};
}

CrSlice(wchar_t) crStateFixMark(struct CrState *state, CrSlice(wchar_t) mark) {
	if (mark.p < state->buf.p)
		mark.p = 0;
	if (mark.p >= state->buf.p + state->buf.s)
		mark.p = state->buf.p + state->buf.s;

	if (mark.p + mark.s > state->buf.p + state->buf.s)
		mark.s = state->buf.p + state->buf.s - mark.p;

	return mark;
}

struct CrErr crStateMatch(
	struct CrState *state,
	CrSlice(wchar_t) *out,
	CrSlice(wchar_t) pattern
) {
	struct CrErr err = crRegexMatch(out, state->mark, pattern);
	if (err.kind)
		err.tok = *state->tok;

	return (struct CrErr){0};
}

int crRunStr(struct CrState *out, char *buf, char *prog) {
	setlocale(LC_ALL, "");

	struct CrGroup g;
	struct CrErr e = crParseStr(prog, &g);
	if (e.kind) {
		crErrPrint(stderr, e);
		fprintf(stderr, "\n");
		return 1;
	}

	struct CrState s;
	crStateInit(&s);
	crStateSetBuf(&s, buf);

	e = crEval(&s, &g);
	if (e.kind) {
		crErrPrint(stderr, e);
		fprintf(stderr, "\n");
		return 1;
	}

	crFreeGroup(&g);

	*out = s;
	return 0;
}
