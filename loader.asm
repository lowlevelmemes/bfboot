bits 16
org 0x7c00

cli
jmp 0x0:main

%include "a20_enabler.inc"
%include "disk.inc"
%include "gdt.inc"

; -- main --

main:
xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov sp, 0x7bf0
sti

call enable_a20

cli
lgdt [GDT]
mov eax, cr0
or al, 1
mov cr0, eax
jmp 0x08:.pmode16
.pmode16:
mov ax, 0x10
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov eax, cr0
and al, 0xfe
mov cr0, eax
jmp 0x0000:.umode
.umode:
xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
sti

mov eax, 1              ; load aux routines
mov ebx, 0x7e00
mov ecx, 3
call read_sectors

mov eax, 4              ; load payload
mov ebx, 0x100000
mov ecx, sectcount - 4
call read_sectors

mov ah, 0x01            ; disable VGA cursor
mov cx, 0x2607
int 0x10

jmp stage2

times 510-($-$$) db 0
dw 0xaa55

stage2:

; -- pmode 32 --

cli
mov eax, cr0
or al, 1
mov cr0, eax
jmp 0x18:.pmode
.pmode:
bits 32
mov ax, 0x20
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

call init_screen

; -- clear data memory --

mov edi, 0x100000 + sectcount * 512
xor eax, eax
mov ecx, 0x400000       ; zero 16 megs
rep stosd

; -- prepare vector regs --

mov ebx, 0x100000 + sectcount * 512 + 0x10000
mov ecx, putchar
mov edx, getchar

; -- jump to code --

jmp 0x100000

%include "aux.inc"

times 2048-($-$$) db 0

incbin "bfboot.tmp.bin"

sectcount       equ     (($-$$) / 512) + 1
times   (sectcount * 512)-($-$$)    db  0
