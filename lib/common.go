package creed

const (
	kindString = iota
	kindNumber
	kindCommand
	kindLGroup
	kindRGroup
)

type token struct {
	value string
	kind  int
	pos   int
}

type Sel struct {
	Index, Length int
}
