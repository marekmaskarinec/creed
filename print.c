#include <creed.h>

void crLocPrint(FILE *f, struct CrLoc loc) {
	fprintf(f, "%d:%d: ", loc.lno, loc.cno);
}

void crErrPrint(FILE *f, struct CrErr err) {
	crLocPrint(f, err.tok.loc);

	switch (err.kind) {
	case CrErrNull:
		fprintf(f, "not an error.");
		break;
	case CrErrUnexpectedToken:
		fprintf(f, "unexpected token.");
		break;
	case CrErrUnterminatedGroup:
		fprintf(f, "unterminated group.");
		break;
	case CrErrStackOverflow:
		fprintf(f, "stack overflow.");
		break;
	case CrErrUnboundSymbol:
		fprintf(f, "unbound symbol.");
		break;
	case CrErrStackUnderflow:
		fprintf(f, "stack underflow.");
		break;
	case CrErrTypeError:
		fprintf(f, "type error.");
		break;
	case CrErrRegexCompile:
		fprintf(f, "regex compile error.");
		break;
	defult:
		fprintf(f, "can't print this error");
		break;
	}
}

void crSymPrint(FILE *f, struct CrSym sym) {
	fprintf(f, ",sym(%.*s, %d)", (int)sym.d.s, sym.d.p, sym.h);
}

void crTokPrint(FILE *f, struct CrTok tok) {
	fprintf(f, ",token(");
	crLocPrint(f, tok.loc);

	switch (tok.kind) {
	case CrTokNull:
		fprintf(f, "null");
		break;
	case CrTokEOF:
		fprintf(f, "EOF");
		break;
	case CrTokGroupBegin:
		fprintf(f, "{\n");
		crGroupPrint(f, tok.group);
		fprintf(f, "}");
		break;
	case CrTokGroupEnd:
		fprintf(f, "}");
		break;
	case CrTokString:
		fprintf(f, "%.*ls", (int)tok.str.s + 1, tok.str.p);
		break;
	case CrTokNumber:
		fprintf(f, "%d", tok.num);
		break;
	case CrTokSymbol:
		crSymPrint(f, tok.sym);
		break;
	}

	fprintf(f, ")");
}

void crValPrint(FILE *f, struct CrVal v) {
	switch (v.kind) {
	case CrValNull:
		fprintf(f, "NULL");
		break;
	case CrValStr:
		fprintf(f, "%.*ls", (int)v.str.s + 1, v.str.p);
		break;
	case CrValNum:
		fprintf(f, "%d", v.num);
		break;
	case CrValSym:
		fprintf(f, "%.*s", (int)v.sym.d.s + 1, v.sym.d.p);
		break;
	case CrValGroup:
		fprintf(f, ",group");
		break;
	}
}

void crGroupPrint(FILE *f, struct CrGroup group) {
	crTokPrint(f, *group.tok);
	fprintf(f, "\n");

	if (group.next) crGroupPrint(f, *group.next);
}
