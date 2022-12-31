#ifndef CREED_H
#define CREED_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECKOUT(e) do { if ((e).kind != CrErrNull) { printf("%s:%d\n", __FILE__, __LINE__); return e; } } while (0)
#define ASSERT(a, e, t) do { if ((a)) { CHECKOUT( ((struct CrErr) { (e), (t) }) ); } } while (0)

#define STACK_SIZE 4096

#define CrSliceDeclare(T) struct CrSlice__##T { T *p; size_t s; }
CrSliceDeclare(char);
CrSliceDeclare(wchar_t);

#define CrSlice(T) struct CrSlice__##T

struct CrHashMapCell {
	uint32_t hash;
	void *p;
};

struct CrHashMap {
	struct CrHashMapCell *cells;
	size_t size;
	void (*freeFn)(struct CrHashMap *, struct CrHashMapCell *);
};

enum CrErrKind {
	CrErrNull,
	CrErrUnexpectedToken,
	CrErrUnterminatedGroup,
	CrErrStackOverflow,
	CrErrUnboundSymbol,
	CrErrStackUnderflow,
	CrErrTypeError,
	CrErrRegexCompile,
	CrErrFileError,
	CrErrMathError,
};

struct CrLoc {
	int cno;
	int lno;
	int idx;
};

enum CrTokKind {
	CrTokNull,
	CrTokEOF,
	CrTokGroupBegin,
	CrTokGroupEnd,
	CrTokString,
	CrTokNumber,
	CrTokSymbol,
	CrTokQuote,
};

struct CrSym {
	CrSlice(char) d;
	unsigned h;
};

struct CrTok;

struct CrParGroup {
	struct CrTok *tok;
	struct CrParGroup *next;
};

struct CrGroup {
	struct CrTok **toks;
	int len;
};

struct CrTok {
	union {
		CrSlice(char) raw;
		CrSlice(wchar_t) str;
		int num;
		struct CrGroup group;
		struct CrSym sym;
	};
	enum CrTokKind kind;
	struct CrLoc loc;
};

struct CrErr {
	enum CrErrKind kind;
	struct CrTok tok;
};

struct CrLex {
	char *buf;
	int bufsiz;
	struct CrTok tok;
	struct CrLoc loc;
};

enum CrValKind {
	CrValNull,
	CrValStr,
	CrValNum,
	CrValGroup,
	CrValSym
};

struct CrVal {
	union {
		CrSlice(wchar_t) str;
		float num;
		struct CrGroup group;
		struct CrSym sym;
	};
	enum CrValKind kind;
};

struct CrState {
	CrSlice(wchar_t) buf;
	CrSlice(wchar_t) mark;

	struct CrHashMap builtins;
	struct CrHashMap syms;

	struct CrVal stackBase[STACK_SIZE];
	struct CrVal *stack;

	// currently evaluated token/group
	struct CrTok *tok;
	struct CrGroup *group;
};

typedef struct CrErr (*CrBuiltin)(struct CrState *);

void crFatal(char *msg, ...);
unsigned crHash(CrSlice(char) s);
char *crWsToMb(CrSlice(wchar_t) s);
CrSlice(wchar_t) crStrDup(CrSlice(wchar_t) s);
char *crReadAll(const char *path);

size_t crUTF8Decode(wchar_t *out, const char *s_);
CrSlice(wchar_t) crUTF8ToSlice(const char *ip, size_t is);

struct CrErr crParse(struct CrLex *lex, struct CrGroup *out);
struct CrErr crParseStr(char *str, struct CrGroup *out);
struct CrErr crParseFile(const char *path, struct CrGroup *out);

struct CrErr crLexNext(struct CrLex *lex);

void crLocPrint(FILE *f, struct CrLoc loc);
void crErrPrint(FILE *f, struct CrErr err);
void crSymPrint(FILE *f, struct CrSym sym);
void crTokPrint(FILE *f, struct CrTok tok);
void crGroupPrint(FILE *f, struct CrGroup group);
void crValPrint(FILE *f, struct CrVal v);

void crStateInit(struct CrState *state);
void crStateSetBuf(struct CrState *state, char *s);
void crStateAddBuiltin(struct CrState *state, char *name, struct CrErr (*fn)(struct CrState *));
struct CrErr crStatePush(struct CrState *state, struct CrVal val);
struct CrErr crStatePushTok(struct CrState *state, struct CrTok *tok);
void crStateSubst(struct CrState *state, CrSlice(wchar_t) mark, CrSlice(wchar_t) text);
struct CrErr crStatePop(struct CrState *state, struct CrVal *out);
struct CrErr crStatePopTyped(struct CrState *state, struct CrVal *out, enum CrValKind type);
CrSlice(wchar_t) crStateFixMark(struct CrState *state, CrSlice(wchar_t) mark);
struct CrErr crStateMatch(struct CrState *state, CrSlice(wchar_t) *out, CrSlice(wchar_t) pattern);
int crRunStr(struct CrState *out, char *buf, char *prog);

void crAttachBuiltins(struct CrState *state);

struct CrErr crEval(struct CrState *state, struct CrGroup *group);

void crHashMapInit(struct CrHashMap *hm);
void *crHashMapGet(struct CrHashMap *hm, uint32_t hash);
void crHashMapSet(struct CrHashMap *hm, uint32_t hash, void *ptr);
void *crHashMapGetStr(struct CrHashMap *hm, char *str);
void crHashMapSetStr(struct CrHashMap *hm, char *str, void *ptr);

void crFreeHashMap(struct CrHashMap *h);
void crFreeSym(struct CrSym *s);
void crFreeGroup(struct CrGroup *g);
void crFreeParGroup(struct CrParGroup *g);
void crFreeTok(struct CrTok *t);
void crFreeLex(struct CrLex *lex);
void crFreeVal(struct CrVal *v);
void crFreeState(struct CrState *s);

void *crDupMem(void *p, size_t s);
struct CrHashMap *crDupHashMap(struct CrHashMap *t, struct CrHashMap *h);
struct CrSym *crDupSym(struct CrSym *t, struct CrSym *s);
struct CrGroup *crDupGroup(struct CrGroup *t, struct CrGroup *g);
struct CrTok *crDupTok(struct CrTok *tgt, struct CrTok *t);
struct CrLex *crDupLex(struct CrLex *t, struct CrLex *lex);
struct CrVal *crDupVal(struct CrVal *t, struct CrVal *v);

#endif
