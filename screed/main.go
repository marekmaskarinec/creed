package main

import (
	"fmt"
	"io"
	"os"

	"github.com/marekmaskarinec/creed/lib"
)

func help() {
	fmt.Fprintf(os.Stderr,
		`screed - stream wrapper around creed
usage: creed command`)
}

func main() {
	if len(os.Args) != 2 || os.Args[1] == "-h" {
		help()
		os.Exit(1)
	}

	dat, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintf(os.Stderr, "fatal: %v\n", err)
		os.Exit(1)
	}

	ins := creed.NewInstance(string(dat), os.Stderr)
	err = ins.ExecCommand(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "creed error: %v\n", err)
		os.Exit(1)
	}
	fmt.Println(string(ins.Buf))
}
