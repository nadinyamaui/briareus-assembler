; Templates are editable. No placeholder is silently supplied as user intent.
proc action_template uses rbx rsi
    mov rsi,templateTable
.find:
    mov rdx,[rsi]
    test rdx,rdx
    jz .fallback
    invoke lstrcmpW,operationText,rdx
    test eax,eax
    jz .found
    add rsi,24
    jmp .find
.found:
    mov ebx,[rsi+16]
    cmp ebx,1
    je .session
    cmp ebx,2
    je .repo
    cmp ebx,3
    je .pull
    cmp ebx,4
    je .errand
    fastcall bbegin,requestText,524288
    fastcall bchar,123
    jmp .suffix
.session:
    fastcall bbegin,requestText,524288
    fastcall btext,sessionPrefix
    fastcall bquote,sessionText
    jmp .suffix
.repo:
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    jmp .suffix
.pull:
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    fastcall btext,prPrefix
    jmp .number
.errand:
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    fastcall btext,<',"prNumber":',0>
.number:
    cmp word [numberText],0
    je .null
    fastcall btext,numberText
    jmp .suffix
.null:
    fastcall btext,<'null',0>
.suffix:
    fastcall btext,qword [rsi+8]
    fastcall bchar,125
    ret
.fallback:
    invoke lstrcpyW,requestText,emptyObject
    ret
endp
