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
}
