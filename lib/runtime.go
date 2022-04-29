package creed

import (
	"container/list"
	"errors"
	"fmt"
	"strconv"
)

var commandsReady = false

func popAny(stack *list.List) (interface{}, error) {
	front := stack.Front()
	if front == nil {
		return nil, errors.New("popAny: stack underflow")
	}

	stack.Remove(stack.Front())

	return front.Value, nil
}

func popString(stack *list.List) (string, error) {
	v, err := popAny(stack)
	if err != nil {
		return "", fmt.Errorf("popString: %w", err)
	}

	if _, valid := v.(string); !valid {
		return "", fmt.Errorf("required type string, got type %T", v)
	}

	return v.(string), nil
}

func popNumber(stack *list.List) (int, error) {
	v, err := popAny(stack)
	if err != nil {
		return 0, fmt.Errorf("popNumber: %w", err)
	}

	if _, valid := v.(int); !valid {
		return 0, errors.New("popString: type error")
	}

	return v.(int), nil
}

func popGroup(stack *list.List) ([]token, error) {
	v, err := popAny(stack)
	if err != nil {
		return []token{}, fmt.Errorf("popGroup: %w", err)
	}

	if g, valid := v.([]token); valid {
		return g, nil
	}

	return []token{}, errors.New("popGroup: type error")
}

func assertStackLen(stack *list.List, l int) error {
	for n := stack.Front(); n != nil && l > 0; n = n.Next() {
		l--
	}

	if l != 0 {
		return fmt.Errorf("stack assertion failed")
	}

	return nil
}

func (ins *Instance) execToks(toks []token) error {
	stack := list.List{}

	for i := 0; i < len(toks); i++ {
		tok := toks[i]

		switch tok.kind {
		case kindString:
			stack.PushFront(tok.value)

		case kindNumber:
			v, err := strconv.Atoi(tok.value)
			if err != nil {
				return err
			}

			stack.PushFront(v)

		case kindLGroup:
			nest := 1
			start := i + 1
			for i++; nest > 0; i++ {
				if toks[i].kind == kindLGroup {
					nest++
				} else if toks[i].kind == kindRGroup {
					nest--
				}
			}
			i--

			stack.PushFront(toks[start:i])

		case kindCommand:
			if fun, exists := commands[toks[i].value]; exists {
				err := fun(ins, &stack)
				if err != nil {
					return err
				}
			} else {
				return fmt.Errorf("(%d) Unknown function name %s.", toks[i].pos, toks[i].value)
			}
		}
	}

	return nil
}
