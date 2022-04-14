package creed

import "unicode"

func lex(command string) []token {
	toks := make([]token, 0, len(command)/3)

	runes := []rune(command)
	for i := 0; i < len(runes); i++ {
		r := runes[i]

		tok := token{pos: i}
		switch r {
		case ' ':
			continue

		case '"':
			tok.kind = kindString
			for i++; i < len(runes) && runes[i] != '"'; i++ {
				tok.value += string(runes[i])
			}

		case '{':
			tok.kind = kindLGroup
			tok.value = "{"

		case '}':
			tok.kind = kindRGroup
			tok.value = "}"

		default:
			if unicode.IsDigit(r) {
				tok.kind = kindNumber
				tok.value = string(r)
				for i++; i < len(runes) && unicode.IsDigit(runes[i]); i++ {
					tok.value += string(runes[i])
				}
			} else {
				tok.kind = kindCommand
				for ; i < len(runes) && runes[i] != ' '; i++ {
					tok.value += string(runes[i])
				}
			}
		}

		toks = append(toks, tok)
	}

	return toks
}
