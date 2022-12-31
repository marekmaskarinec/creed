#include <creed.h>

struct CrErr crEval(struct CrState *state, struct CrGroup *group) {
	state->group = group;

	for (int i=0; i < group->len; ++i) {
		struct CrTok *tok = group->toks[i];
		state->tok = group->toks[i];

		switch (tok->kind) {
		case CrTokQuote:
		case CrTokGroupBegin:
		case CrTokString:
		case CrTokNumber:
			CHECKOUT(crStatePushTok(state, tok));
			break;
		case CrTokSymbol:;
			const unsigned h = tok->sym.h;

			if (crHashMapGet(&state->syms, h))
				CHECKOUT(crEval(state, crHashMapGet(&state->syms, h)));
			else if (crHashMapGet(&state->builtins, h))
				CHECKOUT(((CrBuiltin)crHashMapGet(&state->builtins, h))(state));
			else
				return (struct CrErr){ CrErrUnboundSymbol, *tok };

			break;

		default:
			break;
		}
	}

	return (struct CrErr){0};
}
