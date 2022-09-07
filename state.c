#include <regex.h>
#include <string.h>

#include <creed.h>

void crStateInit(struct CrState *state) {
	crHashMapInit(&state->builtins);
	crHashMapInit(&state->syms);

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
		state->stack->group = tok->group;
		break;

	case CrTokString:
		state->stack->kind = CrValStr;
		state->stack->str = tok->str;
		break;

	case CrTokNumber:
		state->stack->kind = CrValNum;
		state->stack->num = tok->num;
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

	memcpy(p, state->buf.p, markIdx * sizeof(wchar_t));
	memcpy(p + markIdx, text.p, text.s * sizeof(wchar_t));
	memcpy(
	       p + markIdx + text.s,
	       markEnd,
	       (bufEnd - markEnd + 1) * sizeof(wchar_t));

	free(state->buf.p);
	state->buf.p = p;
	state->buf.s = s;

	state->mark.p = state->buf.p + markIdx;
	state->mark.s = text.s;
}

struct CrErr crStatePop(struct CrState *state, struct CrVal *out) {
	ASSERT(state->stack <= state->stackBase, CrErrStackUnderflow, *state->tok);

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

static
int wcIndex(const char *buf, int index) {
	int out = 0;
	for (int i=0; i < index; ++out)
		i += mblen(&buf[i], index - i);

	return out;
}

static
struct CrErr match(regex_t reg, const char *buf, CrSlice(wchar_t) *out) {
	regmatch_t m = {0};
	if (regexec(&reg, buf, 1, &m, 0))
		return (struct CrErr){0};

	out->p = NULL + wcIndex(buf, m.rm_so);
	out->s = wcIndex(buf, m.rm_eo) - (int)out->p;

	return (struct CrErr){0};
}

struct CrErr crStateMatch(struct CrState *state, CrSlice(wchar_t) *out, CrSlice(wchar_t) pattern) {
	char *pat = crWsToMb(pattern);

	regex_t preg;
	ASSERT(regcomp(&preg, pat, REG_EXTENDED), CrErrRegexCompile, *state->tok);

	char *buf = crWsToMb((CrSlice(wchar_t)){
		.p = state->mark.p,
		.s = state->buf.p + state->buf.s - state->mark.p
	});

	CrSlice(wchar_t) m = {0};
	CHECKOUT(match(preg, buf, &m));
	if (m.s == 0 && state->mark.p > state->buf.p) {
		free(buf);
		buf = crWsToMb((CrSlice(wchar_t)){
			.p = state->buf.p,
			.s = state->mark.p - state->buf.p
		});

		CHECKOUT(match(preg, buf, &m));
	}

	out->p = m.s == 0 ? 0 : state->buf.p + (int)m.p;
	out->s = m.s;

	regfree(&preg);
	free(buf);
	free(pat);

	return (struct CrErr){0};
}
