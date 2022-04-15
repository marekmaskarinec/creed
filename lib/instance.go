package creed

import (
	"io"
	"fmt"
	"regexp"
)

type Change struct {
	Sel   Sel
	value string
}

type Instance struct {
	// The instance's content
	Buf []rune
	// main sel
	Sel Sel

	// Buffer holding all changes, which is popped from upon the undo command
	UndoBuffer []Change
	// Writer that is written to when CREED needs to print something
	Writer io.Writer
}

func NewInstance(src string, writer io.Writer) Instance {
	if !commandsReady {
		initCommands()
	}

	return Instance{
		Buf: []rune(src),
		Writer: writer,
	}
}

func (ins *Instance) ExecCommand(command string) error {
	tokens := lex(command)
	return ins.execToks(tokens)
}

func (ins *Instance) subst(sel Sel, value string) {
	buf := make([]rune, 0, len(ins.Buf)-sel.Length+len(value))
	buf = append(buf, ins.Buf[:sel.Index]...)
	buf = append(buf, []rune(value)...)
	buf = append(buf, ins.Buf[sel.Index+sel.Length:]...)
	ins.Buf = buf

	ins.UndoBuffer = append(ins.UndoBuffer, Change{sel, value})
}

func (ins *Instance) fixSel() {
	if ins.Sel.Index < 0 {
		ins.Sel.Index = 0
	}

	if ins.Sel.Index >= len(ins.Buf) {
		ins.Sel.Index = len(ins.Buf) - 1
		ins.Sel.Length = 0
	}

	if ins.Sel.Index+ins.Sel.Length >= len(ins.Buf) {
		ins.Sel.Length = len(ins.Buf) - ins.Sel.Index - 1
	}

	if ins.Sel.Length < 0 {
		ins.Sel.Length = 0
	}
}

func (ins *Instance) match(t string) ([]int, error) {
	regex, err := regexp.Compile(t)
	if err != nil {
		return []int{}, fmt.Errorf("regex error: %w", err)
	}
	match := regex.FindStringIndex(string(ins.Buf[ins.Sel.Index + ins.Sel.Length:]))
	if len(match) == 0 {
		match = regex.FindStringIndex(string(ins.Buf[:ins.Sel.Index + ins.Sel.Length]))
	} else {
		match[0] += ins.Sel.Index + ins.Sel.Length
		match[1] += ins.Sel.Index + ins.Sel.Length
	}

	if len(match) == 0 {
		return []int{}, nil
	}

	return match, nil
}

func (ins *Instance) selectLine() {
	for ins.Sel.Index > 0 && ins.Buf[ins.Sel.Index - 1] != '\n' {
		ins.Sel.Index--
	}

	ins.Sel.Length = 0
	for ins.Sel.Index + ins.Sel.Length < len(ins.Buf) &&
		ins.Buf[ins.Sel.Index + ins.Sel.Length] != '\n' {
		ins.Sel.Length++
	}
}

func (ins *Instance) moveLinesDown(n int) {
	for n > 0 {
		for ;
			ins.Sel.Index < len(ins.Buf) && ins.Buf[ins.Sel.Index] != '\n';
			ins.Sel.Index++ { }
		ins.Sel.Index++

		n--
	}

	ins.fixSel()
}

func (ins *Instance) moveLinesUp(n int) {
	for n > 0 {
		for ;
			ins.Sel.Index > 0 && ins.Buf[ins.Sel.Index] != '\n';
			ins.Sel.Index-- { }
		ins.Sel.Index--

		n--
	}

	ins.fixSel()
}

func (ins *Instance) selSlice() []rune {
	return ins.Buf[ins.Sel.Index:ins.Sel.Index+ins.Sel.Length]
}
