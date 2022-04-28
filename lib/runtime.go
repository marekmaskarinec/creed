package creed

import (
	"container/list"
	"errors"
	"fmt"
	"strconv"
	"regexp"
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

var commands = map[string]func(ins *Instance, stack *list.List) error{
	"d": func(ins *Instance, stack *list.List) error {
		ins.subst(ins.Sel, "")

		return nil
	},
	"s": func(ins *Instance, stack *list.List) error {
		str, err := popString(stack)
		if err != nil {
			return fmt.Errorf("command s: %w", err)
		}

		ins.subst(ins.Sel, str)

		return nil
	},
	"a": func(ins *Instance, stack *list.List) error {
		str, err := popString(stack)
		if err != nil {
			return fmt.Errorf("command a: %w", err)
		}

		ins.subst(Sel{ins.Sel.Index + ins.Sel.Length, 0}, str)
		ins.Sel.Length += len(str)

		return nil
	},
	"p": func(ins *Instance, stack *list.List) error {
		str, err := popString(stack)
		if err != nil {
			return fmt.Errorf("command p: %w", err)
		}

		ins.subst(Sel{ins.Sel.Index, 0}, str)
		ins.Sel.Index += len(str)

		return nil
	},
	"++": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command ++: %w", err)
		}

		ins.Sel.Index += n
		ins.fixSel()

		return nil
	},
	"--": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command --: %w", err)
		}

		ins.Sel.Index -= n
		ins.fixSel()

		return nil
	},
	"%%": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command %%: %w", err)
		}

		ins.Sel.Index = n
		ins.fixSel()

		return nil
	},
	"+l": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command +l: %w", err)
		}

		ins.Sel.Length += n
		ins.fixSel()

		return nil
	},
	"-l": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command -l: %w", err)
		}

		ins.Sel.Length -= n
		ins.fixSel()

		return nil
	},
	"%l": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command %l: %w", err)
		}

		ins.Sel.Length = n
		ins.fixSel()

		return nil
	},
	"/": func(ins *Instance, stack *list.List) error {
		t, err := popString(stack)
		if err != nil {
			return fmt.Errorf("command /: %w", err)
		}

		match, err := ins.match(t)
		if err != nil {
			return fmt.Errorf("command /: %w", err)
		}

		if len(match) == 0 {
			return fmt.Errorf("command /: no match")
		}

		ins.Sel.Index = match[0]
		ins.Sel.Length = match[1] - match[0]

		return nil
	},
	",": func(ins *Instance, stack *list.List) error {
		t, err := popString(stack)
		if err != nil {
			return fmt.Errorf("command ,: %w", err)
		}

		match, err := ins.match(t)
		if err != nil {
			return fmt.Errorf("command ,: %w", err)
		}

		if len(match) == 0 {
			return fmt.Errorf("command ,: no match")
		}

		if match[0] < ins.Sel.Index {
			ins.Sel.Length += ins.Sel.Index - match[0]
			ins.Sel.Index = match[0]
		} else {
			ins.Sel.Length = match[1] - ins.Sel.Index
		}

		return nil
	},
	"#": func(ins* Instance, stack *list.List) error {
		ins.selectLine()

		return nil
	},
	"+#": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command +#: %w", err)
		}

		ins.moveLinesDown(n)
		ins.selectLine()

		return nil
	},
	"%#": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command %#: %w", err)
		}

		ins.Sel.Index = 0
		ins.moveLinesDown(n - 1)
		ins.selectLine()

		return nil
	},
	"-#": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command -#: %w", err)
		}

		ins.moveLinesDown(n)
		ins.selectLine()

		return nil
	},
	"dup": func(ins *Instance, stack *list.List) error {
		v, err := popAny(stack)
		if err != nil {
			return fmt.Errorf("command dup: %w", err)
		}

		stack.PushFront(v)
		stack.PushFront(v)

		return nil
	},
	"drop": func(ins *Instance, stack *list.List) error {
		_, err := popAny(stack)
		if err != nil {
			return fmt.Errorf("command drop: %w", err)
		}

		return nil
	},
	"swap": func(ins *Instance, stack *list.List) error {
		v1, err := popAny(stack)
		if err != nil {
			return fmt.Errorf("command swap: %w", err)
		}

		v2, err := popAny(stack)
		if err != nil {
			return fmt.Errorf("command swap: %w", err)
		}

		stack.PushFront(v1)
		stack.PushFront(v2)

		return nil
	},
	"nip": func(ins *Instance, stack *list.List) error {
		keep, err := popAny(stack)
		if err != nil {
			return fmt.Errorf("commadn nip: %w", err)
		}

		_, err = popAny(stack)
		if err != nil {
			return fmt.Errorf("command nip: %w", err)
		}

		stack.PushFront(keep)

		return nil
	},
	"rot": func(ins *Instance, stack *list.List) error {
		vals := [3]interface{}{nil, nil, nil}
		var err error

		for i:=0; i < 3; i++ {
			vals[i], err = popAny(stack)
			if err != nil {
				return fmt.Errorf("command rot: %w", err)
			}
		}

		stack.PushFront(vals[1])
		stack.PushFront(vals[0])
		stack.PushFront(vals[2])

		return nil
	},
	"tuck": func(ins *Instance, stack *list.List) error {
		if err := assertStackLen(stack, 2); err != nil {
			return fmt.Errorf("command tuck: %w", err)
		}
		v := stack.Front().Value
		stack.InsertAfter(v, stack.Front().Next())

		return nil
	},
	"over": func(ins *Instance, stack *list.List) error {
		if err := assertStackLen(stack, 2); err != nil {
			return fmt.Errorf("command tuck: %w", err)
		}
		stack.PushFront(stack.Front().Next().Value)
		return nil
	},
	"neg": func(ins *Instance, stack *list.List) error {
		n, err := popNumber(stack)
		if err != nil {
			return fmt.Errorf("command neg: %w", err)
		}

		stack.PushFront(n * -1)
		return nil
	},
	"dump": func(ins *Instance, stack *list.List) error {
		fmt.Fprintf(ins.Writer, "<%d> ", stack.Len())
		for e:=stack.Front(); e != nil; e = e.Next() {
			fmt.Fprintf(ins.Writer, "%v ", e.Value)
		}

		fmt.Fprintf(ins.Writer, "\n")

		return nil
	},
}

func commandX(ins *Instance, stack *list.List) error {
	g, err := popGroup(stack)
	if err != nil {
		return fmt.Errorf("command x: %w", err)
	}

	t, err := popString(stack)
	if err != nil {
		return fmt.Errorf("command x: %w", err)
	}

	re, err := regexp.Compile(t)
	if err != nil {
		return fmt.Errorf("regex error: %w", err)
	}

	oldBuf := ins.Buf
	oldSel := ins.Sel
	ins.Buf = make([]rune, ins.Sel.Length)
	copy(ins.Buf, oldBuf[ins.Sel.Index:ins.Sel.Index+ins.Sel.Length])

	ins.Sel.Index = 0
	ins.Sel.Length = 0
	for {
		match := re.FindStringIndex(string(ins.Buf[ins.Sel.Index+ins.Sel.Length:]))
		if len(match) == 0 {
			break
		}

		ins.Sel.Index = match[0] + ins.Sel.Index + ins.Sel.Length
		ins.Sel.Length = match[1] - match[0]
		if err := ins.execToks(g); err != nil {
			return fmt.Errorf("command x: %w", err)
		}
	}

	ins.Sel = oldSel

	value := string(ins.Buf)
	ins.Buf = oldBuf
	ins.subst(ins.Sel, value)

	return nil
}

func commandG(ins *Instance, stack *list.List) error {
	g, err := popGroup(stack)
	if err != nil {
		return fmt.Errorf("command g: %w", err)
	}

	t, err := popString(stack)
	if err != nil {
		return fmt.Errorf("command g: %w", err)
	}

	re, err := regexp.Compile(t)
	if err != nil {
		return fmt.Errorf("command g: regex error: %w", err)
	}

	if match := re.FindStringIndex(string(ins.selSlice())); len(match) != 0 {
		ins.Sel.Index = match[0]
		ins.Sel.Length = match[1] - match[0]

		if err := ins.execToks(g); err != nil {
			return fmt.Errorf("command g: %w", err)
		}
	}

	return nil
}

func initCommands() {
	commands["x"] = commandX
	commands["g"] = commandG
	commandsReady = true
}
