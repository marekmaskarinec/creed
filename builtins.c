#include <stdio.h>

#include <creed.h>

static
struct CrErr hello_world(struct CrState *state) {
	printf("hello world\n");

	return (struct CrErr){ .kind = CrErrNull };
}

static
struct CrErr dump(struct CrState *state) {
	fprintf(stderr, "<%ld> ", state->stack - state->stackBase + 1);

	for (struct CrVal *v=state->stack; v >= state->stackBase; --v) {
		crValPrint(stderr, *v);
		fprintf(stderr, " ");
	}

	fprintf(stderr, "\n");

	return (struct CrErr){ CrErrNull };
}

static
struct CrErr drop(struct CrState *state) {
	struct CrVal val;
	CHECKOUT(crStatePop(state, &val));
	return (struct CrErr){0};
}

static
struct CrErr s(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	crStateSubst(state, state->mark, v.str);
	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr a(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	crStateSubst(state,
		(CrSlice(wchar_t)){
			.p = state->mark.p + state->mark.s,
			.s = 0
		},
		v.str);
	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr p(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	crStateSubst(state,
		(CrSlice(wchar_t)){
			.p = state->mark.p,
			.s = 0
		},
		v.str);
	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr PERCENT(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValNum));

	state->mark.p = state->buf.p + v.num;

	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr PERCENTDOT(struct CrState *state) {
	CHECKOUT(crStatePush(state, (struct CrVal){
		.kind = CrValNum,
		.num = state->mark.p - state->buf.p
	}));

	return (struct CrErr){0};
}

static
struct CrErr PERCENTPERCENT(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValNum));

	state->mark.s = v.num;

	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr PERCENTPERCENTDOT(struct CrState *state) {
	CHECKOUT(crStatePush(state, (struct CrVal){
		.kind = CrValNum,
		.num = state->mark.s
	}));

	return (struct CrErr){0};
}

static
struct CrErr PERCENTSLASH(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	CrSlice(wchar_t) m;
	CHECKOUT(crStateMatch(state, &m, v.str));
	state->mark = m;

	return (struct CrErr){0};
}

static
struct CrErr PERCENTPERCENTSLASH(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	CrSlice(wchar_t) m;
	CHECKOUT(crStateMatch(state, &m, v.str));

	if (m.p > state->mark.p) {
		state->mark.s = m.p - state->mark.p + m.s;
	} else {
		state->mark.s += state->mark.p - m.p;
		state->mark.p = m.p;
	}

	return (struct CrErr){0};
}

static
struct CrErr ATPERCENTDOT(struct CrState *state) {
	CHECKOUT(crStatePush(state, (struct CrVal){
		.kind = CrValStr,
		.str = crStrDup(state->mark)
	}));

	return (struct CrErr){0};
}

static
struct CrErr apply(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValGroup));

	CHECKOUT(crEval(state, &v.group));

	return (struct CrErr){0};
}

static
struct CrErr branch(struct CrState *state) {
	struct CrVal g2;
	CHECKOUT(crStatePopTyped(state, &g2, CrValGroup));
	struct CrVal g1;
	CHECKOUT(crStatePopTyped(state, &g1, CrValGroup));
	struct CrVal n;
	CHECKOUT(crStatePopTyped(state, &n, CrValNum));

	if (n.num)
		CHECKOUT(crEval(state, &g1.group));
	else
		CHECKOUT(crEval(state, &g2.group));

	return (struct CrErr){0};
}

static
struct CrErr AT(struct CrState *state) {
	CHECKOUT(crEval(state, state->group));
	return (struct CrErr){0};
}

static
struct CrErr awas(struct CrState *state) {
	struct CrVal g;
	CHECKOUT(crStatePopTyped(state, &g, CrValGroup));

	CrSlice(wchar_t) buf = state->buf;
	CrSlice(wchar_t) mark = state->mark;
	state->buf = crStrDup(state->mark);
	state->mark = state->buf;

	CHECKOUT(crEval(state, &g.group));
	CrSlice(wchar_t) ret = state->buf;
	state->buf = buf;
	state->mark = mark;
	crStateSubst(state, state->mark, ret);

	free(ret.p);

	return (struct CrErr){0};
}

void crAttachBuiltins(struct CrState *state) {
	crStateAddBuiltin(state, "hello-world" , hello_world              );
	crStateAddBuiltin(state, "dump"        , dump                     );
	crStateAddBuiltin(state, "drop"        , drop                     );
	crStateAddBuiltin(state, "s"           , s                        );
	crStateAddBuiltin(state, "a"           , a                        );
	crStateAddBuiltin(state, "p"           , p                        );
	crStateAddBuiltin(state, "%"           , PERCENT                  );
	crStateAddBuiltin(state, "%."          , PERCENTDOT               );
	crStateAddBuiltin(state, "%%"          , PERCENTPERCENT           );
	crStateAddBuiltin(state, "%%."         , PERCENTPERCENTDOT        );
	crStateAddBuiltin(state, "%/"          , PERCENTSLASH             );
	crStateAddBuiltin(state, "%%/"         , PERCENTPERCENTSLASH      );
	crStateAddBuiltin(state, "@%."         , ATPERCENTDOT             );
	crStateAddBuiltin(state, "apply"       , apply                    );
	crStateAddBuiltin(state, "@"           , AT                       );
	crStateAddBuiltin(state, "branch"      , branch                   );
	crStateAddBuiltin(state, "awas"        , awas                     );
}
