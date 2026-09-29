; All transport code executes off the UI thread. Globals are immutable while busy.
proc validate_connection uses rsi rbx
    mov rsi,tokenText
    cmp dword [rsi],00720062h
    jne .badToken
    cmp dword [rsi+4],005f006dh
    jne .badToken
    add rsi,8
    mov ebx,43
.token:
    lodsw
    cmp ax,'A'
    jb .other
    cmp ax,'Z'
    jbe .ok
    cmp ax,'a'
    jb .other
    cmp ax,'z'
    jbe .ok
.other:
    cmp ax,'0'
    jb .special
    cmp ax,'9'
    jbe .ok
.special:
    cmp ax,'_'
    je .ok
    cmp ax,'-'
    jne .badToken
.ok:
    dec ebx
    jnz .token
    cmp word [rsi],0
    jne .badToken
    invoke RtlZeroMemory,urlParts,104
    mov dword [urlParts],104
    mov dword [urlParts+16],-1
    mov dword [urlParts+32],-1
    mov dword [urlParts+48],-1
    mov dword [urlParts+64],-1
    mov dword [urlParts+80],-1
    mov dword [urlParts+96],-1
    invoke WinHttpCrackUrl,serverText,0,0,urlParts
    test eax,eax
    jz .badAddress
    cmp dword [urlParts+20],2
    jne .badAddress
    cmp dword [urlParts+32],0
    je .badAddress
    cmp dword [urlParts+32],255
    ja .badAddress
    cmp word [urlParts+36],0
    je .badAddress
    cmp dword [urlParts+48],0
    jne .badAddress
    cmp dword [urlParts+64],0
    jne .badAddress
    cmp dword [urlParts+96],0
    jne .badAddress
    mov rcx,qword [urlParts+72]
    cmp dword [urlParts+80],0
    je .host
    invoke lstrcmpW,rcx,slashText
    test eax,eax
    jz .host
    invoke lstrcmpW,qword [urlParts+72],basePath
    test eax,eax
    jz .host
    invoke lstrcmpW,qword [urlParts+72],basePathSlash
    test eax,eax
    jnz .badAddress
.host:
    mov r8d,dword [urlParts+32]
    inc r8d
    invoke lstrcpynW,hostText,qword [urlParts+24],r8
    xor eax,eax
    ret
.badAddress:
    mov rax,addressError
    ret
.badToken:
    mov rax,tokenError
    ret
endp

if ~ defined TEST_MODE
proc http_worker uses rbx rsi rdi r12 r13 r14 r15, unused
    xor r12d,r12d
    xor r13d,r13d
    xor r14d,r14d
    mov [netError],0
    mov [httpStatus],0
    mov word [errorText],0
    mov word [responseText],0
    invoke WinHttpOpen,agentText,4,0,0,0
    mov r12,rax
    test rax,rax
    jz .systemError
    invoke WinHttpSetTimeouts,r12,10000,10000,30000,180000
    movzx r8d,word [urlParts+36]
    invoke WinHttpConnect,r12,hostText,r8,0
    mov r13,rax
    test rax,rax
    jz .systemError
    invoke WinHttpOpenRequest,r13,[reqMethod],reqPath,0,0,0,00800000h
    mov r14,rax
    test rax,rax
    jz .systemError
    ; Disable cookies, redirects and automatic authentication.
    invoke WinHttpSetOption,r14,63,disabledFeatures,4
    test eax,eax
    jz .systemError
    invoke WinHttpSetOption,r14,77,highAutologon,4
    test eax,eax
    jz .systemError
    invoke lstrcpyW,headerText,headerPrefix
    invoke lstrcatW,headerText,tokenText
    invoke lstrcatW,headerText,headerSuffix
    xor ebx,ebx
    cmp [reqMethod],postMethod
    jne .send
    invoke WideCharToMultiByte,65001,80h,requestText,-1,requestBytes,1048576,0,0
    test eax,eax
    jz .systemError
    lea ebx,[eax-1]
.send:
    invoke WinHttpSendRequest,r14,headerText,-1,requestBytes,rbx,rbx,0
    test eax,eax
    jz .systemError
    invoke WinHttpReceiveResponse,r14,0
    test eax,eax
    jz .systemError
    mov [querySize],4
    invoke WinHttpQueryHeaders,r14,20000013h,0,httpStatus,querySize,0
    test eax,eax
    jz .systemError
    cmp [httpStatus],300
    jb .mime
    cmp [httpStatus],400
    jb .redirect
.mime:
    mov [querySize],256
    invoke WinHttpQueryHeaders,r14,1,0,mimeText,querySize,0
    test eax,eax
    jz .nonjson
    mov rsi,mimeText
.mimeEnd:
    cmp word [rsi],';'
    je .terminateMime
    cmp word [rsi],0
    je .compareMime
    add rsi,2
    jmp .mimeEnd
.terminateMime:
    mov word [rsi],0
.compareMime:
    invoke lstrcmpiW,mimeText,jsonMime
    test eax,eax
    jnz .nonjson
    xor ebx,ebx
.read:
    cmp ebx,4194303
    jae .oversize
    mov r8d,4194303
    sub r8d,ebx
    lea rdx,[responseBytes+rbx]
    invoke WinHttpReadData,r14,rdx,r8,bytesRead
    test eax,eax
    jz .systemError
    cmp [bytesRead],0
    je .decode
    add ebx,[bytesRead]
    jmp .read
.decode:
    mov byte [responseBytes+rbx],0
    invoke MultiByteToWideChar,65001,8,responseBytes,rbx,responseText,4194303
    test eax,eax
    jz .nonjson
    mov word [responseText+rax*2],0
    cmp [httpStatus],200
    jb .httpError
    cmp [httpStatus],300
    jae .httpError
    jmp .cleanup
.httpError:
    mov [netError],1
    invoke wsprintfW,errorText,httpErrorFormat,[httpStatus]
    jmp .cleanup
.redirect:
    mov rbx,redirectError
    jmp .namedError
.oversize:
    mov rbx,oversizeError
    jmp .namedError
.nonjson:
    mov rbx,jsonError
.namedError:
    mov [netError],1
    invoke lstrcpyW,errorText,rbx
    jmp .cleanup
.systemError:
    invoke GetLastError
    mov [netError],1
    invoke wsprintfW,errorText,networkErrorFormat,rax
.cleanup:
    test r14,r14
    jz .connection
    invoke WinHttpCloseHandle,r14
.connection:
    test r13,r13
    jz .session
    invoke WinHttpCloseHandle,r13
.session:
    test r12,r12
    jz .done
    invoke WinHttpCloseHandle,r12
.done:
    invoke RtlZeroMemory,headerText,2048
    invoke PostMessageW,[mainWindow],WM_APP+1,0,0
    xor eax,eax
    ret
endp
else
include '../tests/mock-worker.inc'
end if

; Credentials are saved only after discovery and capability validation succeed.
proc save_connection
if defined TEST_MODE
    mov [credentialSaved],1
    ret
else
    invoke RtlZeroMemory,credential,80
    invoke lstrcpyW,credentialBlob,serverText
    invoke lstrlenW,serverText
    inc eax
    lea rdx,[credentialBlob+rax*2]
    mov [credentialTokenPtr],rdx
    invoke lstrcpyW,rdx,tokenText
    mov rax,[credentialTokenPtr]
    sub rax,credentialBlob
    add eax,96
    mov dword [credential+4],1
    mov qword [credential+8],credentialTarget
    mov dword [credential+32],eax
    mov qword [credential+40],credentialBlob
    mov dword [credential+48],2
    mov qword [credential+72],credentialUser
    invoke CredWriteW,credential,0
    mov [credentialSaved],eax
    invoke RtlZeroMemory,credentialBlob,8192
    ret
end if
endp

proc restore_connection uses rbx rsi
if defined TEST_MODE
    ret
else
    invoke CredReadW,credentialTarget,1,0,credentialPtr
    test eax,eax
    jz .done
    mov rbx,[credentialPtr]
    mov ecx,[rbx+32]
    cmp ecx,100
    jb .free
    cmp ecx,4096
    ja .free
    test ecx,1
    jnz .free
    mov rsi,[rbx+40]
    ; Enforce two NUL-terminated strings within the credential blob.
    mov edx,ecx
    shr edx,1
    xor ecx,ecx
.scan:
    cmp ecx,edx
    jae .free
    cmp word [rsi+rcx*2],0
    je .serverEnd
    inc ecx
    jmp .scan
.serverEnd:
    cmp ecx,1900
    jae .free
    lea eax,[ecx+49]
    cmp eax,edx
    jne .free
    cmp word [rsi+rax*2-2],0
    jne .free
    lea rax,[rsi+rcx*2+2]
    mov [credentialTokenPtr],rax
    invoke lstrcpyW,serverText,rsi
    invoke lstrcpynW,tokenText,[credentialTokenPtr],48
.free:
    invoke CredFree,rbx
.done:
    ret
end if
endp

