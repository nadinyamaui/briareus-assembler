; UTF-16 JSON parser. Indices are 1-based, zero means failure/missing.
; Each 32-byte token: start:qword, length:dword, kind:dword, next:dword.
; kind: object=1 array=2 string=3 primitive=4. End is exclusive.
proc json_parse uses rbx, source
    mov [jpos],rcx
    mov [jcount],1
    mov [jdepth],0
    mov [jfailed],0
    fastcall jvalue
    mov ebx,eax
    fastcall jspace
    cmp word [rcx],0
    jne .bad
    cmp [jfailed],0
    jne .bad
    mov eax,ebx
    ret
.bad:
    xor eax,eax
    ret
endp

proc jspace
    mov rcx,[jpos]
.loop:
    movzx eax,word [rcx]
    cmp eax,32
    je .skip
    cmp eax,9
    je .skip
    cmp eax,10
    je .skip
    cmp eax,13
    jne .done
.skip:
    add rcx,2
    jmp .loop
.done:
    mov [jpos],rcx
    ret
endp

proc jvalue uses rbx rsi rdi r12
    inc [jdepth]
    cmp [jdepth],64
    ja .bad
    cmp [jcount],32768
    jae .bad
    fastcall jspace
    mov ebx,[jcount]
    inc [jcount]
    mov esi,ebx
    shl rsi,5
    add rsi,jtokens
    mov [rsi],rcx
    mov dword [rsi+12],4
    cmp ax,'{'
    je .object
    cmp ax,'['
    je .array
    cmp ax,'"'
    je .string
    cmp ax,'t'
    je .true
    cmp ax,'f'
    je .false
    cmp ax,'n'
    je .null
    ; JSON number grammar, without converting or losing precision.
    cmp ax,'-'
    jne .integer
    add rcx,2
.integer:
    movzx eax,word [rcx]
    cmp ax,'0'
    je .zero
    cmp ax,'1'
    jb .bad
    cmp ax,'9'
    ja .bad
.digits:
    add rcx,2
    movzx eax,word [rcx]
    cmp ax,'0'
    jb .fraction
    cmp ax,'9'
    jbe .digits
    jmp .fraction
.zero:
    add rcx,2
.fraction:
    cmp word [rcx],'.'
    jne .exponent
    add rcx,2
    cmp word [rcx],'0'
    jb .bad
    cmp word [rcx],'9'
    ja .bad
.fracloop:
    add rcx,2
    cmp word [rcx],'0'
    jb .exponent
    cmp word [rcx],'9'
    jbe .fracloop
.exponent:
    mov ax,[rcx]
    or ax,32
    cmp ax,'e'
    jne .primitive
    add rcx,2
    mov ax,[rcx]
    cmp ax,'+'
    je .sign
    cmp ax,'-'
    jne .expdigits
.sign:
    add rcx,2
.expdigits:
    cmp word [rcx],'0'
    jb .bad
    cmp word [rcx],'9'
    ja .bad
.exploop:
    add rcx,2
    cmp word [rcx],'0'
    jb .primitive
    cmp word [rcx],'9'
    jbe .exploop
.primitive:
    mov [jpos],rcx
    jmp .finish
.true:
    cmp dword [rcx+2],00750072h
    jne .bad
    cmp word [rcx+6],'e'
    jne .bad
    add rcx,8
    jmp .primitive
.false:
    cmp dword [rcx+2],006c0061h
    jne .bad
    cmp dword [rcx+6],00650073h
    jne .bad
    add rcx,10
    jmp .primitive
.null:
    cmp dword [rcx+2],006c0075h
    jne .bad
    cmp word [rcx+6],'l'
    jne .bad
    add rcx,8
    jmp .primitive
.object:
    mov dword [rsi+12],1
    mov r12d,'}'
    jmp .container
.array:
    mov dword [rsi+12],2
    mov r12d,']'
.container:
    add [jpos],2
    fastcall jspace
    cmp ax,r12w
    je .close
.item:
    cmp dword [rsi+12],1
    jne .arrayvalue
    fastcall jspace
    cmp ax,'"'
    jne .bad
    fastcall jvalue
    test eax,eax
    jz .bad
    fastcall jspace
    cmp ax,':'
    jne .bad
    add [jpos],2
.arrayvalue:
    fastcall jvalue
    test eax,eax
    jz .bad
    fastcall jspace
    cmp ax,r12w
    je .close
    cmp ax,','
    jne .bad
    add [jpos],2
    jmp .item
.close:
    add [jpos],2
    jmp .finish
.string:
    mov dword [rsi+12],3
    add rcx,2
.char:
    movzx eax,word [rcx]
    cmp ax,'"'
    je .stringend
    cmp ax,32
    jb .bad
    add rcx,2
    cmp ax,92
    jne .char
    movzx eax,word [rcx]
    add rcx,2
    cmp ax,'u'
    je .unicode
    cmp ax,'"'
    je .char
    cmp ax,92
    je .char
    cmp ax,'/'
    je .char
    cmp ax,'b'
    je .char
    cmp ax,'f'
    je .char
    cmp ax,'n'
    je .char
    cmp ax,'r'
    je .char
    cmp ax,'t'
    je .char
    jmp .bad
.unicode:
    mov edi,4
.hex:
    movzx eax,word [rcx]
    cmp ax,'0'
    jb .bad
    cmp ax,'9'
    jbe .hexok
    or ax,32
    cmp ax,'a'
    jb .bad
    cmp ax,'f'
    ja .bad
.hexok:
    add rcx,2
    dec edi
    jnz .hex
    jmp .char
.stringend:
    add rcx,2
    mov [jpos],rcx
.finish:
    mov rax,[jpos]
    sub rax,[rsi]
    shr rax,1
    mov [rsi+8],eax
    mov eax,[jcount]
    mov [rsi+16],eax
    dec [jdepth]
    mov eax,ebx
    ret
.bad:
    mov [jfailed],1
    dec [jdepth]
    xor eax,eax
    ret
endp

; Object lookup (keys in the mobile contract are unescaped ASCII).
proc json_get uses rbx rsi rdi r12, object,key
    test ecx,ecx
    jz .missing
    mov r12,rdx
    mov ebx,ecx
    shl rcx,5
    cmp dword [jtokens+rcx+12],1
    jne .missing
    mov edi,dword [jtokens+rcx+16]
    inc ebx
.loop:
    cmp ebx,edi
    jae .missing
    mov esi,ebx
    shl rsi,5
    mov rcx,qword [jtokens+rsi]
    add rcx,2
    mov rdx,r12
.compare:
    mov ax,[rdx]
    test ax,ax
    jz .endkey
    cmp ax,[rcx]
    jne .next
    add rdx,2
    add rcx,2
    jmp .compare
.endkey:
    cmp word [rcx],'"'
    jne .next
    lea eax,[rbx+1]
    ret
.next:
    inc ebx
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .loop
.missing:
    xor eax,eax
    ret
endp

; Copy a token to a bounded wide string, unescaping JSON strings.
proc json_text uses rsi rdi rbx r12, token,dest,capacity
    mov rdi,rdx
    lea r12,[rdx+r8*2-2]
    test ecx,ecx
    jz .finishall
    shl rcx,5
    mov rsi,qword [jtokens+rcx]
    mov ebx,dword [jtokens+rcx+8]
    cmp dword [jtokens+rcx+12],3
    jne .raw
    add rsi,2
    sub ebx,2
.unescape:
    test ebx,ebx
    jz .finishall
    cmp rdi,r12
    jae .finishall
    lodsw
    dec ebx
    cmp ax,92
    jne .put
    lodsw
    dec ebx
    cmp ax,'u'
    je .unicode
    cmp ax,'n'
    jne .r
    mov ax,10
.r:
    cmp ax,'r'
    jne .t
    mov ax,13
.t:
    cmp ax,'t'
    jne .b
    mov ax,9
.b:
    cmp ax,'b'
    jne .f
    mov ax,8
.f:
    cmp ax,'f'
    jne .put
    mov ax,12
    jmp .put
.unicode:
    xor edx,edx
    mov ecx,4
.hex:
    lodsw
    shl edx,4
    cmp ax,'9'
    jbe .digit
    or ax,32
    sub ax,39
.digit:
    sub ax,'0'
    movzx eax,ax
    or edx,eax
    loop .hex
    sub ebx,4
    mov eax,edx
    test ax,ax
    jnz .put
    mov ax,0fffdh
.put:
    stosw
    jmp .unescape
.raw:
    test ebx,ebx
    jz .finishall
    cmp rdi,r12
    jae .finishall
    lodsw
    stosw
    dec ebx
    jmp .raw
.finishall:
    mov word [rdi],0
    mov rax,rdi
    ret
endp

; Bounded UTF-16 builder. Flag overflow instead of sending truncated JSON.
proc bbegin
    mov [bptr],rcx
    lea rax,[rcx+rdx*2-2]
    mov [bend],rax
    mov [boverflow],0
    mov word [rcx],0
    ret
endp
proc bchar
    mov rax,[bptr]
    cmp rax,[bend]
    jae .full
    mov [rax],cx
    add rax,2
    mov word [rax],0
    mov [bptr],rax
    ret
.full:
    mov [boverflow],1
    ret
endp
proc btext uses rsi rbx
    mov rsi,rcx
    mov rbx,rcx
.loop:
    lodsw
    test ax,ax
    jz .done
    cmp ax,10
    jne .char
    lea rcx,[rbx+2]
    cmp rsi,rcx
    je .crlf
    cmp word [rsi-4],13
    je .char
.crlf:
    fastcall bchar,13
    mov ax,10
.char:
    movzx ecx,ax
    fastcall bchar,rcx
    jmp .loop
.done:
    ret
endp
proc bquote uses rsi rbx
    mov rsi,rcx
    fastcall bchar,34
.loop:
    lodsw
    test ax,ax
    jz .finishall
    movzx ebx,ax
    cmp ax,'"'
    je .escape
    cmp ax,92
    je .escape
    cmp ax,32
    jb .control
.put:
    fastcall bchar,rbx
    jmp .loop
.escape:
    fastcall bchar,92
    jmp .put
.control:
    fastcall btext,unicodePrefix
    mov ecx,ebx
    shr ecx,4
    movzx ecx,word [hexChars+rcx*2]
    fastcall bchar,rcx
    and ebx,15
    movzx ecx,word [hexChars+rbx*2]
    fastcall bchar,rcx
    jmp .loop
.finishall:
    fastcall bchar,34
    ret
endp
