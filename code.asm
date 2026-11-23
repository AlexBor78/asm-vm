jmp start

_data:
msg: dq "Fuck the word!", 0h
var1: dd FF00FF00h
var2: dd 00ff00ffh
var3: db 00FFh

start:
push var1
push var2
__print_stack

; COMMENT

add_:
mod
__print_stack
not
__print_stack
push FF00FF00FF00FFh
__print_stack
xor
__print_stack

output:
;jmp end_
push 48h
putc

push 65h
putc

push 6ch
putc
push 6ch
putc

push 6fh
putc

push 2ch
putc
push 20h
putc

push 57h
putc
push 6fh
putc
push 72h
putc
push 6ch
putc
push 64h
putc
push 21h
putc

push 0ah
putc

end_:
ret
