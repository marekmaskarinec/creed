CREED - concatenative regex-based editor

CREED is a concatenative language for editing text. It heavily relies on regex.
CREED is implemented as a go library, which can be used inside editors. See
screed - a program applying a creed command to stdin and then printing the
result to stdout.

Language description:

A CREED instance contains two main things - a sel and a buffer. Buffer is an
array of UTF32 characters (yes, creed handles unicode). Sel describes an area
of text in the buffer using a start index and a length. Sel can never be out of
bounds - commands will move it and trim it so it's always valid. Currently
CREED uses go's regex, but I will write my own if I ever have time for it.
There are 3 types in CREED - numbers, "strings" and {groups}. The syntax is
very similar to other concatenative languages like forth.

Command list:

Editing commands:

d          delete text in sel. like "" s
s ( t -- ) substitute text in sel with t
a ( t -- ) append text after sel
p ( t -- ) prepend text before sel

Sel commands:

#  ( -- )   select the whole line
-# ( n -- ) move sel N lines up, select current line
+# ( n -- ) move sel N lines down, select current line
%# ( n -- ) select the whole nth line

++ ( n -- ) increment sel position by N characters
-- ( n -- ) decrement sel position by N characters
%% ( n -- ) set absolute sel position  

+l ( n -- ) increment sel length by N characters
-l ( n -- ) decrement sel length by N characters
%l ( n -- ) set absolute sel length

+w ( n -- ) move sel N words forward - todo
-w ( n -- ) move sel N words backward - todo

/  ( t -- ) set sel to the first (from sel) longest match of regex t
,  ( t -- ) set sel length to the end of the first longest match of regex t

Loops and conditionals:

x ( tg -- ) foreach match of t run g
y ( tg -- ) between matches of t run g - todo
g ( tg -- ) if sel contains t, run g - todo
v ( tg -- ) if sel doesn't contain t, run g - todo
j ( ng -- ) if n is not a zero, run g - todo

Stack operation:

like in forth.

dup - todo
drop - todo
swap - todo
nip - todo
rot - todo
tuck - todo
over - todo
roll - todo
pick - todo

Operators:

neg ( n -- n ) negates a number - todo

Misc:

help ( -- ) help - todo

