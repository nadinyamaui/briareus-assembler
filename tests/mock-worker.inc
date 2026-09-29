; Compile-time fixture transport. Never linked into the release executable.
proc http_worker uses rbx, unused
    mov [netError],0
    mov [httpStatus],200
    mov rbx,fixtureEmpty
    cmp [reqKind],1
    je .discovery
    cmp [reqKind],2
    je .catalog
    cmp [reqKind],3
    je .projects
    invoke lstrcmpW,operationText,opSessions
    test eax,eax
    jz .sessions
    invoke lstrcmpW,operationText,opSession
    test eax,eax
    jz .session
    invoke lstrcmpW,operationText,opStart
    test eax,eax
    jz .session
    invoke lstrcmpW,operationText,opMessage
    test eax,eax
    jz .session
    invoke lstrcmpW,operationText,opPulls
    test eax,eax
    jz .pulls
    invoke lstrcmpW,operationText,opPull
    test eax,eax
    jz .pull
    invoke lstrcmpW,operationText,opFiles
    test eax,eax
    jz .files
    jmp .copy
.discovery:
    mov rbx,fixtureDiscovery
    cmp word [tokenText+8],'r'
    jne .copy
    mov rbx,fixtureReadOnly
    jmp .copy
.catalog:
    mov rbx,fixtureOperations
    jmp .copy
.projects:
    mov rbx,fixtureProjects
    jmp .copy
.sessions:
    mov rbx,fixtureSessions
    jmp .copy
.session:
    mov rbx,fixtureSession
    jmp .copy
.pulls:
    mov rbx,fixturePulls
    jmp .copy
.pull:
    mov rbx,fixturePull
    jmp .copy
.files:
    mov rbx,fixtureFiles
.copy:
    invoke lstrcpyW,responseText,rbx
    cmp [reqKind],3
    jb .notify
    invoke CreateFileW,testLogPath,40000000h,1,0,4,80h,0
    cmp rax,-1
    je .notify
    mov rbx,rax
    invoke SetFilePointer,rbx,0,0,2
    invoke WideCharToMultiByte,65001,0,requestText,-1,requestBytes,1048576,0,0
    test eax,eax
    jz .close
    dec eax
    invoke WriteFile,rbx,requestBytes,rax,bytesRead,0
    invoke WriteFile,rbx,testLogNewline,1,bytesRead,0
.close:
    invoke CloseHandle,rbx
.notify:
    invoke PostMessageW,[mainWindow],WM_APP+1,0,0
    xor eax,eax
    ret
endp
