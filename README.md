# CREED - concatenative regex-based editor

CREED is a concatenative language for simple text processing scripts.

## Language description

A CREED state contains three things - the mark, the stack and the buffer.
The mark is a pair of an index and a length both referencing the buffer. The
buffer is a string. The stack is the stack used by CREED programs.

Creed supports three base data types. Numbers, "text", symbols and
{groups}, which are anonymous functions that can also be used as arrays.

You can bind a group to a symbol using bind. Example:

```
{ dup multiply } "square" bind
```

Upon writing a symbol, apply is used on that symbols content.

## Builtin funcions

These are functions included in the core language, however there is a builtin
library providing many more functions.

notation: `( input -- output )`
	* `t` is text
	* `n` is a number
	* `g` is a group
	* `r` is a regex (still text)

### Insert commands:

* `s ( t -- )` - substitute the marked text with t
* `a ( t -- )` - append text after the marked text
* `p ( t -- )` - prepend text before the marked text

### Mark commands:

* `%    ( n -- )` - set absolute mark position
* `%.   ( -- n )` - push the mark index to the stack

* `%%   ( n -- )` - set absolute mark length
* `%%.  ( -- n )` - push the mark length to the stack

* `%/   ( r -- )` - set the mark to the match of r
* `%%/  ( r -- )` - extend the mark to the end of match of r

* `@%.  ( -- t )` - pushes the marked content to the stack

### Flow control:

* `apply  ( g -- )` -   apply g
* `@      ( -- )` -     apply the current group recursively
* `branch ( ngg -- )` - apply the first g if n is non zero, else apply the second g
* `while  ( gg -- )` -  while the top of the stack after running group 1 is 1,
                        runs group 2
* `awas   ( g -- )` -   apply with altered state - applies the group with the marked
                        text as the buffer, and then substitutes it back

### Stack operation:

like in forth.

* `dup`
* `drop`
* `swap`
* `rot`
* `tuck`
* `over`
* `roll`
* `pick`

### IO:

* `parse   ( t -- g )` - parses a string into a group

* `read    ( t -- )` -   reads contents of a file onto the stack
* `write   ( tt -- )` -  writes text from the stack to a file, rewrites
* `writea  ( tt -- )` -  writes text from the stack to a file, appends
* `!       ( t -- t )` -  executes a shell command and pushes the `stdout`

### Math:

* `neg      ( n -- n )`
* `plus     ( nn -- n )`
* `minus    ( nn -- n )`
* `divide   ( nn -- n )`
* `multiply ( nn -- n )`
* `modulo   ( nn -- n )`

### Logic:

* `and`
* `or`
* `equal`
* `lesser`
* `greater`

### groups:

* `append ( gg -- g )` - appends groups
* `take   ( gn -- g )` - pushes a group with the first n elements
* `drop   ( gn -- g )` - pushes a group with the last n elements
* `group  ( an -- g )` - groups the first n items on the stack

### Misc:

* `help ( -- )` - help
* `dump ( -- )` - dumps the stack content
