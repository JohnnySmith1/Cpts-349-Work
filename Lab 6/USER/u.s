.globl _syscall,_main,_exit,_getcs,auto_start

export auto_start
auto_start:
    call _main

! if ever return, exit(0)
	push  #0
    call  _exit

_syscall:
        int    80
        ret
_getcs:
	mov   ax,cs
	ret
