init_screen:
pusha

mov edi, 0xb8000								; Point to video memory

; Draw prompt (at 0,0)

mov ah, byte [CursorAttributes]	; Get cursor attributes
mov al, ' '
stosw

; Fill the screen with spaces

mov ah, byte [CharAttributes]	; Get char attributes
mov al, ' '

mov ecx, 0x07CF
rep stosw

; Set up the variables

mov dword [CursorLocation], 0xb8000

popa
ret

putchar:

pusha

test al, al						; Is 0x00 (null)?
jz .done						; Ignore

cmp al, 0x0A					; Is 0x0A?
je .next_line

cmp al, 0x08					; Is 0x08?
je .backspace

mov ebx, dword [CursorLocation]
inc ebx
mov ah, byte [CharAttributes]
mov byte [ebx], ah						; Attributes first to avoid 'ghosting'
dec ebx
mov byte [ebx], al							; Print character
inc ebx
inc ebx
cmp ebx, 0xb8fa0						; End of video memory?
je .scroll_screen_down
mov dword [CursorLocation], ebx
call draw_cursor				; Draw cursor
jmp .done

.next_line:
call get_cursor_position
xor ah, ah								; Set X to 0
cmp al, 24								; Last line?
je .scroll_screen_down					; Scroll screen
inc al									; Y + 1
call set_cursor_position
jmp .done								; Done

.backspace:
call clear_cursor
sub dword [CursorLocation], 2
mov edi, dword [CursorLocation]
mov al, ' '
stosb
mov al, byte [CursorAttributes]
stosb
call draw_cursor
jmp .done

.scroll_screen_down:
call clear_cursor			; Destroy cursor
mov ebx, 0xb8000 + 160						; Next line

.scroll_down_loop:
cmp ebx, 0xb8fa0					; Last char?
je .clear_line
mov eax, dword [ebx]			; Get word
mov dword [ebx-160], eax		; Copy to previous line
add ebx, 4
jmp .scroll_down_loop

.clear_line:
sub ebx, 160
mov dword [CursorLocation], ebx	; New cursor location
mov al, byte [CharAttributes]

.clear_line_loop:
cmp ebx, 0xb8fa0
je .clear_line_done
mov byte [ebx], ' '
inc ebx
mov byte [ebx], al
inc ebx
jmp .clear_line_loop

.clear_line_done:
call draw_cursor			; Restore cursor

.done:
popa
ret

get_cursor_position:

; OUT: AH = cursor X
;      AL = cursor Y

push ebx
push ecx
push edx

mov eax, dword [CursorLocation]
sub eax, 0xb8000
mov ebx, 160					; Divide AX / 160
xor edx, edx
div ebx
and eax, 0x000000ff         	; Clear high 8
push eax						; Push result, for now
mov eax, edx					; Load MOD
mov ebx, 2					; Divide MOD / 2
xor edx, edx
div ebx
mov edx, eax					; Result in DX
pop eax						; Restore AX
mov ah, dl					; Move AH

pop edx
pop ecx
pop ebx

ret

set_cursor_position:

; IN: AH = cursor X
;     AL = cursor Y

pusha

call clear_cursor

push eax						; Save AX
and eax, 0x000000ff         	; Clear high 8
mov ebx, 160					; Multiply Y * 160
mul ebx
mov dword [CursorLocation], eax	; Save for now
pop eax						; Restore AX
and eax, 0xffffff00					; Clear low 8
shr eax, 8					; Shift right 8
add eax, eax					; Multiply X * 2
add dword [CursorLocation], eax	; Add
add dword [CursorLocation], 0xb8000

call draw_cursor		; Draw cursor back

.done:

popa

ret

draw_cursor:

pusha

mov edi, dword [CursorLocation]
mov al, byte [edi]
mov ah, byte [CursorAttributes]

stosw

popa

ret

clear_cursor:

pusha

mov edi, dword [CursorLocation]
mov al, byte [edi]
mov ah, byte [CharAttributes]

stosw

popa

ret

CursorLocation			dd 0xb8000
CharAttributes			db 0x07
CursorAttributes		db 0x70


getchar:

push ebx

cmp dword [.buffer_ptr], 0
jne .dump

; get scancode

.wait:
xor eax, eax
in al, 0x64
and al, 1
test al, al
jnz .ready
jmp .wait

.ready:
in al, 0x60     ; fetch code

cmp al, 0x36
je .shift
cmp al, 0x2a
je .shift
cmp al, 0xb6
je .shift
cmp al, 0xaa
je .shift
cmp al, 58
jae .wait
add eax, dword [.cur_tab]
mov al, byte [eax]

mov ebx, dword [.buffer_ptr]
cmp ebx, 127
ja .wait        ; buffer full, do not accept
call putchar
xchg bx, bx
mov byte [.buffer + ebx], al
inc dword [.buffer_ptr]
cmp al, 0x0a
je .dump
jmp .wait

.dump:
mov eax, dword [.buffer_dmp]
mov al, byte [.buffer + eax]
cmp al, 0x0a
je .dumped
inc dword [.buffer_dmp]
jmp .done

.shift:
mov eax, dword [.last_tab]
xchg dword [.cur_tab], eax
mov dword [.last_tab], eax
jmp .wait

.dumped:
mov dword [.buffer_ptr], 0
mov dword [.buffer_dmp], 0

.done:
pop ebx
ret

.cur_tab dd .ascii_tab
.last_tab dd .ascii_shift_tab

.ascii_tab:
db    0, '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08, 0x09
db    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0a, 0, 'a', 's'
db    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0, '\', 'z', 'x', 'c', 'v'
db    'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '

.ascii_shift_tab:
db    0, '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08, 0x09
db    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x0a, 0, 'A', 'S'
db    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V'
db    'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '

.buffer times 128 db 0
.buffer_ptr dd 0
.buffer_dmp dd 0
