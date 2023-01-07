#include <math.h>
#include <stdio.h>
#include <string.h>

#include <creed.h>

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
	
	crFreeVal(&val);
	return (struct CrErr){0};
}

static
struct CrErr s(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	crStateSubst(state, state->mark, v.str);
	state->mark = crStateFixMark(state, state->mark);

	crFreeVal(&v);
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

	crFreeVal(&v);
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

	crFreeVal(&v);
	return (struct CrErr){0};
}

static
struct CrErr amp(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValNum));

	state->mark.p = state->buf.p + (size_t)v.num;

	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr pmp(struct CrState *state) {
	CHECKOUT(crStatePush(state, (struct CrVal){
		.kind = CrValNum,
		.num = state->mark.p - state->buf.p
	}));

	return (struct CrErr){0};
}

static
struct CrErr aml(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValNum));

	state->mark.s = v.num;

	state->mark = crStateFixMark(state, state->mark);
	return (struct CrErr){0};
}

static
struct CrErr pml(struct CrState *state) {
	CHECKOUT(crStatePush(state, (struct CrVal){
		.kind = CrValNum,
		.num = state->mark.s
	}));

	return (struct CrErr){0};
}

static
struct CrErr mrm(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	CrSlice(wchar_t) m;
	CHECKOUT(crStateMatch(state, &m, v.str));
	if (m.p)
		state->mark = m;

	crFreeVal(&v);
	return (struct CrErr){0};
}

static
struct CrErr emrm(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	CrSlice(wchar_t) m;
	struct CrErr err = crRegexMatch(
		&m,
		(CrSlice(wchar_t)){
			.p = state->mark.p,
			.s = state->buf.s + state->buf.p - state->mark.p
		},
		v.str);
	if (err.kind)
		err.tok = *state->tok;

	if (m.p > state->mark.p) {
		state->mark.s = m.p - state->mark.p + m.s;
	} else {
		state->mark.s += state->mark.p - m.p;
		state->mark.p = m.p;
	}

	crFreeVal(&v);
	return (struct CrErr){0};
}

static
struct CrErr pm(struct CrState *state) {
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

	crFreeVal(&v);

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

	crFreeVal(&g1);
	crFreeVal(&g2);
	return (struct CrErr){0};
}

static
struct CrErr while_(struct CrState *state) {
	struct CrVal g2;
	CHECKOUT(crStatePopTyped(state, &g2, CrValGroup));
	struct CrVal g1;
	CHECKOUT(crStatePopTyped(state, &g1, CrValGroup));

	do {
		CHECKOUT(crEval(state, &g1.group));
	
		struct CrVal v;
		CHECKOUT(crStatePopTyped(state, &v, CrValNum));
		// NOTE(~mrms): we don't need to free v, since it's always going to be a
		// number.
		if (v.num == 0)
			break;

		CHECKOUT(crEval(state, &g2.group));
	} while (1);

	crFreeVal(&g1);
	crFreeVal(&g2);

	return (struct CrErr){0};
}

static
struct CrErr rec(struct CrState *state) {
	CHECKOUT(crEval(state, state->group));
	return (struct CrErr){0};
}

static
struct CrErr awmb(struct CrState *state) {
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

	crFreeVal(&g);
	return (struct CrErr){0};
}

static
struct CrErr dup(struct CrState *state) {
	struct CrVal v, d;
	CHECKOUT(crStatePop(state, &v));
	crDupVal(&d, &v);

	CHECKOUT(crStatePush(state, v));
	CHECKOUT(crStatePush(state, d));

	return (struct CrErr){0};
}

static
struct CrErr swap(struct CrState *state) {
	struct CrVal v1;
	CHECKOUT(crStatePop(state, &v1));
	struct CrVal v2;
	CHECKOUT(crStatePop(state, &v2));

	CHECKOUT(crStatePush(state, v2));
	CHECKOUT(crStatePush(state, v1));

	return (struct CrErr){0};
}

static
struct CrErr rot(struct CrState *state) {
	struct CrVal v1;
	CHECKOUT(crStatePop(state, &v1));
	struct CrVal v2;
	CHECKOUT(crStatePop(state, &v2));
	struct CrVal v3;
	CHECKOUT(crStatePop(state, &v3));

	CHECKOUT(crStatePush(state, v2));
	CHECKOUT(crStatePush(state, v1));
	CHECKOUT(crStatePush(state, v3));

	return (struct CrErr){0};
}

static
struct CrErr tuck(struct CrState *state) {
	struct CrVal v1, v1d;
	CHECKOUT(crStatePop(state, &v1));
	crDupVal(&v1d, &v1);
	
	struct CrVal v2;
	CHECKOUT(crStatePop(state, &v2));

	CHECKOUT(crStatePush(state, v1d));
	CHECKOUT(crStatePush(state, v2));
	CHECKOUT(crStatePush(state, v1));

	return (struct CrErr){0};
}

static
struct CrErr over(struct CrState *state) {
	struct CrVal v1;
	CHECKOUT(crStatePop(state, &v1));
	struct CrVal v2, v2d;
	CHECKOUT(crStatePop(state, &v2));
	crDupVal(&v2d, &v2);

	CHECKOUT(crStatePush(state, v2));
	CHECKOUT(crStatePush(state, v1));
	CHECKOUT(crStatePush(state, v2d));

	return (struct CrErr){0};
}

static
struct CrErr parse(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	char *s = crWsToMb(v.str);

	struct CrGroup g;
	CHECKOUT(crParseStr(s, &g));

	CHECKOUT(crStatePush(state, (struct CrVal){ .group = g, .kind = CrValGroup }));

	free(s);
	crFreeVal(&v);

	return (struct CrErr){0};
}

static
struct CrErr read(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));
	char *s = crWsToMb(v.str);

	char *f = crReadAll(s);
	free(s);

	CHECKOUT(crStatePush(state, (struct CrVal){
		.str = crUTF8ToSlice(f, strlen(f)),
		.kind = CrValStr }));

	free(f);
	crFreeVal(&v);

	return (struct CrErr){0};
}

static
struct CrErr write_(struct CrState *state, char *mode) {
	struct CrVal wpath;
	CHECKOUT(crStatePopTyped(state, &wpath, CrValStr));
	char *path = crWsToMb(wpath.str);

	struct CrVal data;
	CHECKOUT(crStatePop(state, &data));


	FILE *f = fopen(path, mode);
	free(path);
	ASSERT(f == NULL, CrErrFileError, *state->tok);

	crValPrint(f, data);

	fclose(f);

	crFreeVal(&wpath);
	return (struct CrErr){0};
}

static 
struct CrErr write(struct CrState *state) {
	CHECKOUT(write_(state, "w"));
	return (struct CrErr){0};
}

static
struct CrErr writea(struct CrState *state) {
	CHECKOUT(write_(state, "a"));
	return (struct CrErr){0};
}

static
struct CrErr neg(struct CrState *state) {
	struct CrVal n;
	CHECKOUT(crStatePopTyped(state, &n, CrValNum));
	n.num *= -1;

	CHECKOUT(crStatePush(state, n));

	return (struct CrErr){0};
}

#define ARITHMETIC(name, operator) \
static                                            \
struct CrErr name(struct CrState *state) {        \
	struct CrVal a, b;                              \
	CHECKOUT(crStatePopTyped(state, &b, CrValNum)); \
	CHECKOUT(crStatePopTyped(state, &a, CrValNum)); \
                                                  \
	a.num = a.num operator b.num;                   \
                                                  \
	CHECKOUT(crStatePush(state, a));                \
                                                  \
	return (struct CrErr){0};                       \
}

ARITHMETIC(add, +);
ARITHMETIC(sub, -);
ARITHMETIC(mul, *);
ARITHMETIC(and, &&);
ARITHMETIC(or, ||);
ARITHMETIC(eq, ==);
ARITHMETIC(lt, <);
ARITHMETIC(gt, >);
ARITHMETIC(le, <=);
ARITHMETIC(ge, >=);

static
struct CrErr div_(struct CrState *state) {
	struct CrVal a, b;
	CHECKOUT(crStatePopTyped(state, &b, CrValNum));
	CHECKOUT(crStatePopTyped(state, &a, CrValNum));

	ASSERT(b.num == 0, CrErrMathError, *state->tok);

	a.num = a.num / b.num;

	CHECKOUT(crStatePush(state, a));

	return (struct CrErr){0};
}

static
struct CrErr mod(struct CrState *state) {
	struct CrVal a, b;
	CHECKOUT(crStatePopTyped(state, &b, CrValNum));
	CHECKOUT(crStatePopTyped(state, &a, CrValNum));

	ASSERT(b.num == 0, CrErrMathError, *state->tok);

	a.num = fmod(a.num, b.num);

	CHECKOUT(crStatePush(state, a));

	return (struct CrErr){0};
}

static
struct CrErr bind(struct CrState *state) {
	struct CrVal s, g;
	CHECKOUT(crStatePopTyped(state, &s, CrValSym));
	CHECKOUT(crStatePopTyped(state, &g, CrValGroup));

	crHashMapSet(&state->syms,
		s.sym.h,
		crDupGroup(malloc(sizeof(struct CrGroup)), &g.group));

	crFreeVal(&s);
	crFreeVal(&g);

	return (struct CrErr){0};
}

static
struct CrErr shell(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePopTyped(state, &v, CrValStr));

	char *com = crWsToMb(v.str);
	FILE *f = popen(com, "r");

	char str[BUFSIZ];
	char *head = str;
	char c;
	while ((c = fgetc(f)) != EOF) {
		*head = c;
		head++;
	}
	*head = 0;

	CHECKOUT(crStatePush(state, (struct CrVal){
		.str = crUTF8ToSlice(str, head - str),
		.kind = CrValStr
	}));

	crFreeVal(&v);
	free(com);
	pclose(f);

	return (struct CrErr){0};
}	

static
struct CrErr repr(struct CrState *state) {
	char buf[4096];
	FILE *f = fmemopen(buf, sizeof(buf), "w");

	struct CrVal v;
	CHECKOUT(crStatePop(state, &v));

	crValPrint(f, v);

	fclose(f);
	crFreeVal(&v);

	CHECKOUT(crStatePush(state, (struct CrVal){
		.str = crUTF8ToSlice(buf, strlen(buf)),
		.kind = CrValStr
	}));

	return (struct CrErr){0};
}

static
struct CrErr put(struct CrState *state) {
	struct CrVal v;
	CHECKOUT(crStatePop(state, &v));

	crValPrint(stdout, v);

	crFreeVal(&v);

	return (struct CrErr){0};
}

static
struct CrErr awtb(struct CrState *state) {
	struct CrVal g, t;
	CHECKOUT(crStatePopTyped(state, &g, CrValGroup));
	CHECKOUT(crStatePopTyped(state, &t, CrValStr));

	const CrSlice(wchar_t) buf = state->buf;
	const CrSlice(wchar_t) mark = state->mark;
	state->buf = t.str;
	state->mark = t.str;

	CHECKOUT(crEval(state, &g.group));

	t.str = state->buf;
	CHECKOUT(crStatePush(state, t));

	state->buf = buf;
	state->mark = mark;

	crFreeVal(&g);

	return (struct CrErr){0};
}

static
struct CrErr hrm(struct CrState *state) {
	struct CrVal r;
	CHECKOUT(crStatePopTyped(state, &r, CrValStr));
	CrSlice(wchar_t) out;

	CHECKOUT(crStateMatch(state, &out, r.str));
	CHECKOUT(crStatePush(state, (struct CrVal){
		.num = out.s == 0 ? 0 : 1,
		.kind = CrValNum
	}));

	crFreeVal(&r);

	return (struct CrErr){0};
}

static
struct CrErr mall(struct CrState *state) {
	state->mark = state->buf;
	return (struct CrErr){0};
}

static
struct CrErr mend(struct CrState *state) {
	state->mark.s = state->buf.p + state->buf.s - state->mark.p;
	return (struct CrErr){0};
}

static
struct CrErr mbegin(struct CrState *state) {
	state->mark.s = state->mark.s + state->mark.p - state->buf.p;
	state->mark.p = state->buf.p;
	return (struct CrErr){0};
}

static
struct CrErr mcl(struct CrState *state) {
	for (;
		state->mark.p > state->buf.p && *state->mark.p != '\n';
		--state->mark.p
	) ;

	for (;
		state->mark.p + state->mark.s < state->buf.p + state->buf.s;
		++state->mark.s
	) {
		if (*(state->mark.p + state->mark.s) == '\n')
			break;
	}

	return (struct CrErr){0};
}

void crAttachBuiltins(struct CrState *state) {
	crStateAddBuiltin(state, "dump"   , dump   );
	crStateAddBuiltin(state, "drop"   , drop   );
	crStateAddBuiltin(state, "s"      , s      );
	crStateAddBuiltin(state, "a"      , a      );
	crStateAddBuiltin(state, "p"      , p      );
	crStateAddBuiltin(state, "amp"    , amp    );
	crStateAddBuiltin(state, "pmp"    , pmp    );
	crStateAddBuiltin(state, "aml"    , aml    );
	crStateAddBuiltin(state, "pml"    , pml    );
	crStateAddBuiltin(state, "mrm"    , mrm    );
	crStateAddBuiltin(state, "emrm"   , emrm   );
	crStateAddBuiltin(state, "pm"     , pm     );
	crStateAddBuiltin(state, "apply"  , apply  );
	crStateAddBuiltin(state, "rec"    , rec    );
	crStateAddBuiltin(state, "branch" , branch );
	crStateAddBuiltin(state, "while"  , while_ );
	crStateAddBuiltin(state, "awmb"   , awmb   );
	crStateAddBuiltin(state, "dup"    , dup    );
	crStateAddBuiltin(state, "swap"   , swap   );
	crStateAddBuiltin(state, "rot"    , rot    );
	crStateAddBuiltin(state, "tuck"   , tuck   );
	crStateAddBuiltin(state, "over"   , over   );
	crStateAddBuiltin(state, "parse"  , parse  );
	crStateAddBuiltin(state, "read"   , read   );
	crStateAddBuiltin(state, "write"  , write  );
	crStateAddBuiltin(state, "writea" , writea );
	crStateAddBuiltin(state, "neg"    , neg    );
	crStateAddBuiltin(state, "add"    , add    );
	crStateAddBuiltin(state, "sub"    , sub    );
	crStateAddBuiltin(state, "div"    , div_   );
	crStateAddBuiltin(state, "mul"    , mul    );
	crStateAddBuiltin(state, "mod"    , mod    );
	crStateAddBuiltin(state, "and"    , and    );
	crStateAddBuiltin(state, "or"     , or     );
	crStateAddBuiltin(state, "eq"     , eq     );
	crStateAddBuiltin(state, "lt"     , lt     );
	crStateAddBuiltin(state, "gt"     , gt     );
	crStateAddBuiltin(state, "ge"     , ge     );
	crStateAddBuiltin(state, "le"     , le     );
	crStateAddBuiltin(state, "bind"   , bind   );
	crStateAddBuiltin(state, "shell"  , shell  );
	crStateAddBuiltin(state, "repr"   , repr   );
	crStateAddBuiltin(state, "put"    , put    );
	crStateAddBuiltin(state, "awtb"   , awtb   );
	crStateAddBuiltin(state, "hrm"    , hrm    );
	crStateAddBuiltin(state, "mall"   , mall   );
	crStateAddBuiltin(state, "mend"   , mend   );
	crStateAddBuiltin(state, "mbegin" , mbegin );
	crStateAddBuiltin(state, "mcl"    , mcl    );
}

