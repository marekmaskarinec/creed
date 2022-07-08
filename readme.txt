CREED - concatenative regex-based editor

CREED is a concatenative language for simple text processing scripts.

Language description
===============================================================================

A CREED state contains three things - the mark, the stack and the buffer.
The mark is a pair of an index and a length both referencing the buffer. The
buffer is a string. The stack is the stack used by CREED programs.

Creed supports three base data types. Numbers, "text", symbols and
{groups}, which are anonymous functions that can also be used as arrays.

You can bind a group to a symbol using bind. Example:

{ dup multiply } "square" bind

Upon writing a symbol, apply is used on that symbols content.

Builtin funcions
===============================================================================

These are functions included in the core language, however there is a builtin
library providing many more functions.

notation: ( input -- output)
	t is text
	n is a number
	g is a group
	r is a regex (still text)


s ( t -- ) substitute the marked text with t
a ( t -- ) append text after the marked text
p ( t -- ) prepend text before the marked text

Mark commands:

%    ( n -- ) set absolute mark position
%.   ( -- n ) push the mark index to the stack

%%   ( n -- ) set absolute mark length
%%.  ( -- n ) push the mark length to the stack

%/   ( r -- ) set the mark to the match of r
%%/  ( r -- ) extend the mark to the end of match of r

@%.  ( -- )   pushes the marked content to the stack

Flow control:

apply  ( g -- )   apply g
@      ( -- )     apply the current group recursively
branch ( ngg -- ) apply the first g if n is non zero, else apply the second g
awas   ( g -- )   apply with altered state - applies the group with the marked
                  text as the buffer, and then substitutes it back

Stack operation:

like in forth.

dup
drop
swap
rot
tuck
over
roll
pick

IO:

include ( t -- ) includes the symbols defined in a file
eval    ( t -- ) evaluates t, with the current stack

read    ( t -- ) sets the content to the contents of a file. Won't work if the
                 instance is currently not saved. if an unexisting file is passed,
                 CREED won't load anything, but will just set the instance's file
write   ( -- ) writes the content to the last loaded file. if the file doesn't
               exist, creates it.
discard ( -- ) sets the saved flag to true, but doesn't save anything

Math:

neg      ( n -- n )
plus     ( nn -- n )
minus    ( nn -- n )
divide   ( nn -- n )
multiply ( nn -- n )
modulo   ( nn -- n )

Logic:

and
or
equal
lesser
greater

groups:

append ( gg -- g ) appends groups
take   ( gn -- g ) pushes a group with the first n elements
drop   ( gn -- g ) pushes a group with the last n elements
group  ( an -- g ) groups the first n items on the stack

Misc:

help ( -- ) help
dump ( -- ) dumps the stack content
