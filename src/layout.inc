; Native responsive layout, driven solely by the available client area.
; >=1100 px: projects / list / detail. >=800: navigation / detail.
; Below 800: one panel, with explicit navigation preserving control state.
proc layout uses rbx rsi rdi r12 r13 r14 r15
    cmp [h_status],0
    je .done
    invoke GetClientRect,[mainWindow],clientRect
    mov ebx,[clientRect.right]
    mov r15d,[clientRect.bottom]
    test ebx,ebx
    jz .done
    test r15d,r15d
    jz .done
    lea edi,[rbx-32]
    invoke MoveWindow,[h_brand],16,10,rdi,40,TRUE
    invoke MoveWindow,[h_tagline],16,54,rdi,22,TRUE
    lea eax,[r15-48]
    invoke MoveWindow,[h_status],16,rax,rdi,40,TRUE
    cmp [paired],0
    jne .dashboard
    irps control, navProjects navList navDetail {
        invoke ShowWindow,[h_#control],SW_HIDE
    }
    lea esi,[rbx-64]
    cmp esi,570
    jle .pairWidth
    mov esi,570
.pairWidth:
    invoke MoveWindow,[h_serverLabel],32,128,rsi,24,TRUE
    invoke MoveWindow,[h_server],32,158,rsi,36,TRUE
    invoke MoveWindow,[h_tokenLabel],32,218,rsi,24,TRUE
    invoke MoveWindow,[h_token],32,248,rsi,36,TRUE
    invoke MoveWindow,[h_pairHelp],32,308,rsi,96,TRUE
    invoke MoveWindow,[h_connect],32,424,rsi,40,TRUE
    jmp .done
.dashboard:
    ; Connection commands remain reachable in every size class.
    lea eax,[rbx-48]
    xor edx,edx
    mov ecx,3
    div ecx
    mov esi,eax
    cmp esi,180
    jle .toolbarWidth
    mov esi,180
.toolbarWidth:
    invoke MoveWindow,[h_refresh],16,80,rsi,32,TRUE
    lea eax,[rsi+24]
    invoke MoveWindow,[h_forget],rax,80,rsi,32,TRUE
    lea eax,[rsi*2+32]
    invoke MoveWindow,[h_revoke],rax,80,rsi,32,TRUE
    ; Short caption fits 480 px without reducing the font or hit target.
    invoke SetWindowTextW,[h_revoke],<'Revoke token',0>
    mov [layoutMode],0
    mov r14d,128
    mov r12d,536
    lea r13d,[rbx-552]
    cmp ebx,1100
    jge .wide
    mov r14d,164
    ; Navigation tabs share the full available width, independently of commands.
    lea eax,[rbx-48]
    xor edx,edx
    mov ecx,3
    div ecx
    mov esi,eax
    invoke MoveWindow,[h_navProjects],16,124,rsi,32,TRUE
    lea eax,[rsi+24]
    invoke MoveWindow,[h_navList],rax,124,rsi,32,TRUE
    lea eax,[rsi*2+32]
    invoke MoveWindow,[h_navDetail],rax,124,rsi,32,TRUE
    irps control, navProjects navList navDetail {
        invoke ShowWindow,[h_#control],SW_SHOW
    }
    mov [layoutMode],1
    mov r12d,272
    lea r13d,[rbx-288]
    cmp ebx,800
    jge .panes
    mov [layoutMode],2
    mov r12d,16
    lea r13d,[rbx-32]
    jmp .panes
.wide:
    irps control, navProjects navList navDetail {
        invoke ShowWindow,[h_#control],SW_HIDE
    }
.panes:
    mov esi,224
    cmp [layoutMode],0
    je .projectGeometry
    mov esi,240
    cmp [layoutMode],1
    je .projectGeometry
    lea esi,[rbx-32]
.projectGeometry:
    invoke MoveWindow,[h_projectsLabel],16,r14,rsi,28,TRUE
    lea eax,[r14+36]
    mov edi,r15d
    sub edi,eax
    sub edi,60
    invoke MoveWindow,[h_projects],16,rax,rsi,rdi,TRUE
    mov ecx,SW_SHOW
    cmp [layoutMode],0
    je .projectVisibility
    cmp [navPane],0
    je .projectVisibility
    xor ecx,ecx
.projectVisibility:
    mov edi,ecx
    invoke ShowWindow,[h_projectsLabel],rdi
    invoke ShowWindow,[h_projects],rdi
    mov edi,16
    cmp [layoutMode],0
    jne .listGeometry
    mov edi,256
    mov esi,264
.listGeometry:
    invoke MoveWindow,[h_view],rdi,r14,rsi,240,TRUE
    lea eax,[r14+42]
    invoke MoveWindow,[h_search],rdi,rax,rsi,32,TRUE
    lea eax,[r14+86]
    mov ecx,r15d
    sub ecx,eax
    sub ecx,60
    mov r8d,eax
    mov eax,ecx
    invoke MoveWindow,[h_rows],rdi,r8,rsi,rax,TRUE
    mov edi,SW_SHOW
    cmp [layoutMode],0
    je .listVisibility
    cmp [navPane],1
    je .listVisibility
    cmp [layoutMode],1
    jne .hideList
    cmp [navPane],2
    je .listVisibility
.hideList:
    xor edi,edi
.listVisibility:
    irps control, view search rows {
        invoke ShowWindow,[h_#control],rdi
    }
    ; Detail pane's vertical space is shared with a bounded composer.
    invoke MoveWindow,[h_detailTitle],r12,r14,r13,36,TRUE
    lea esi,[r15-92]
    cmp r13d,520
    jge .buttonRow
    sub esi,40
.buttonRow:
    lea edi,[rsi-124]
    invoke MoveWindow,[h_composeLabel],r12,rdi,r13,36,TRUE
    invoke MoveWindow,[h_action],r12,rdi,r13,260,TRUE
    lea eax,[rdi+40]
    invoke MoveWindow,[h_compose],r12,rax,r13,76,TRUE
    lea eax,[r14+44]
    sub edi,eax
    sub edi,12
    invoke MoveWindow,[h_detail],r12,rax,r13,rdi,TRUE
    cmp r13d,520
    jl .wrapButtons
    lea eax,[r13-16]
    xor edx,edx
    mov ecx,3
    div ecx
    mov edi,eax
    invoke MoveWindow,[h_send],r12,rsi,rdi,36,TRUE
    lea eax,[r12+rdi+8]
    invoke MoveWindow,[h_new],rax,rsi,rdi,36,TRUE
    lea eax,[rdi*2+16]
    add eax,r12d
    invoke MoveWindow,[h_advanced],rax,rsi,rdi,36,TRUE
    jmp .apiButtons
.wrapButtons:
    lea edi,[r13-8]
    shr edi,1
    invoke MoveWindow,[h_send],r12,rsi,rdi,36,TRUE
    lea eax,[r12+rdi+8]
    invoke MoveWindow,[h_new],rax,rsi,rdi,36,TRUE
    lea eax,[rsi+40]
    invoke MoveWindow,[h_advanced],r12,rax,r13,36,TRUE
.apiButtons:
    lea edi,[r13-8]
    shr edi,1
    invoke MoveWindow,[h_execute],r12,rsi,rdi,36,TRUE
    lea eax,[r12+rdi+8]
    invoke MoveWindow,[h_back],rax,rsi,rdi,36,TRUE
    mov edi,SW_SHOW
    cmp [layoutMode],2
    jne .detailVisibility
    cmp [navPane],2
    je .detailVisibility
    xor edi,edi
.detailVisibility:
    irps control, detailTitle detail compose {
        invoke ShowWindow,[h_#control],rdi
    }
    mov esi,edi
    cmp [advancedMode],0
    je .messageVisibility
    xor esi,esi
.messageVisibility:
    irps control, composeLabel send new advanced {
        invoke ShowWindow,[h_#control],rsi
    }
    cmp [advancedMode],0
    jne .apiVisibility
    xor edi,edi
.apiVisibility:
    irps control, action execute back {
        invoke ShowWindow,[h_#control],rdi
    }
.done:
    ; A size-class change must not strand keyboard focus in a hidden pane.
    invoke GetFocus
    test rax,rax
    jz .return
    invoke IsWindowVisible,rax
    test eax,eax
    jnz .return
    mov rcx,[h_server]
    cmp [paired],0
    je .focus
    mov rcx,[h_detail]
    cmp [layoutMode],2
    jne .focus
    cmp [navPane],2
    je .focus
    mov rcx,[h_rows]
    cmp [navPane],1
    je .focus
    mov rcx,[h_projects]
.focus:
    invoke SetFocus,rcx
.return:
    ret
endp
