#include <creed.h>

struct CrErr crEval(struct CrState *state, struct CrGroup *group) {
	state->group = group;

	for (; group; group = group->next) {
		state->tok = group->tok;

		switch (group->tok->kind) {
		case CrTokQuote:
		case CrTokGroupBegin:
		case CrTokString:
		case CrTokNumber:
			CHECKOUT(crStatePushTok(state, group->tok));
			break;
		case CrTokSymbol:;
			const unsigned h = group->tok->sym.h;

			if (crHashMapGet(&state->syms, h))
				CHECKOUT(crEval(state, crHashMapGet(&state->syms, h)));
			else if (crHashMapGet(&state->builtins, h))
				CHECKOUT(((CrBuiltin)crHashMapGet(&state->builtins, h))(state));
			else
				return (struct CrErr){ CrErrUnboundSymbol, *group->tok };

			break;

		default:
			break;
		}
	}

	return (struct CrErr){0};
}
