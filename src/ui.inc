proc create_ui
    include 'controls.inc'
    ; The themed dropdown renderer can retain stale glyphs during WM_PRINT.
    ; Use the standard native dropdown renderer for deterministic repainting.
    invoke SetWindowTheme,[h_view],emptyText,emptyText
    invoke SetWindowTheme,[h_action],emptyText,emptyText
    fastcall restore_connection
    invoke SetWindowTextW,[h_server],serverText
    invoke SetWindowTextW,[h_token],tokenText
    fastcall show_state
    fastcall layout
    invoke SetTimer,[mainWindow],1,7000,0
    ret
endp

proc show_state uses rbx
    mov ebx,SW_SHOW
    cmp [paired],0
    je .pair
    mov ebx,SW_HIDE
.pair:
    irps control, serverLabel server tokenLabel token pairHelp connect {
      invoke ShowWindow,[h_#control],rbx
    }
    xor ebx,ebx
    cmp [paired],0
    je .main
    mov ebx,SW_SHOW
.main:
    irps control, projectsLabel projects refresh forget revoke view search rows detailTitle detail composeLabel compose send new advanced {
      invoke ShowWindow,[h_#control],rbx
    }
    xor ebx,ebx
    cmp [paired],0
    je .advanced
    cmp [advancedMode],0
    je .advanced
    mov ebx,SW_SHOW
    irps control, composeLabel send new advanced {
      invoke ShowWindow,[h_#control],SW_HIDE
    }
.advanced:
    irps control, action execute back {
      invoke ShowWindow,[h_#control],rbx
    }
    fastcall update_enabled
    fastcall layout
    ret
endp

proc update_enabled uses rbx
    mov ebx,1
    cmp [busy],0
    je .idle
    xor ebx,ebx
.idle:
    irps control, connect server token projects rows view refresh forget revoke new send execute action advanced back {
      invoke EnableWindow,[h_#control],rbx
    }
    cmp [canManage],0
    jne .done
    invoke EnableWindow,[h_new],FALSE
    invoke EnableWindow,[h_send],FALSE
.done:
    cmp [viewMode],0
    jne .disableSend
    cmp word [sessionText],0
    jne .projectCheck
.disableSend:
    invoke EnableWindow,[h_send],FALSE
.projectCheck:
    cmp word [repoText],0
    jne .composer
    invoke EnableWindow,[h_new],FALSE
.composer:
    xor ebx,ebx
    cmp [canManage],0
    jne .editable
    cmp [advancedMode],0
    jne .editable
    mov ebx,1
.editable:
    invoke SendMessageW,[h_compose],EM_SETREADONLY,rbx,0
    mov rdx,text_composeLabel
    cmp [canManage],0
    jne .boardLabel
    mov rdx,readComposerLabel
    jmp .setLabel
.boardLabel:
    cmp [viewMode],0
    je .setLabel
    mov rdx,boardComposerLabel
.setLabel:
    invoke SetWindowTextW,[h_composeLabel],rdx
    cmp [uncertainWrite],0
    je .return
    invoke EnableWindow,[h_new],FALSE
    invoke EnableWindow,[h_send],FALSE
.return:
    ret
endp

include 'layout.inc'

proc status, text
    mov rdx,rcx
    invoke SetWindowTextW,[h_status],rdx
    ret
endp

; Catalog enforcement applies to read requests too. Return 1 read, 2 write, 0 absent.
proc allowed uses rbx rsi, name
    mov rsi,rcx
    xor ebx,ebx
.loop:
    cmp ebx,[operationCount]
    jae .no
    mov ecx,ebx
    shl ecx,8
    add rcx,operationNames
    invoke lstrcmpW,rcx,rsi
    test eax,eax
    jz .found
    inc ebx
    jmp .loop
.found:
    mov eax,1
    cmp dword [operationReadOnly+rbx*4],1
    je .done
    cmp [canManage],0
    je .no
    mov eax,2
.done:
    ret
.no:
    xor eax,eax
    ret
endp

proc begin_request
    cmp [busy],0
    jne .done
    mov [busy],1
    fastcall update_enabled
    fastcall status,busyText
    invoke CreateThread,0,0,http_worker,0,0,0
    test rax,rax
    jz .failed
    invoke CloseHandle,rax
.done:
    ret
.failed:
    mov [busy],0
    fastcall update_enabled
    fastcall status,<'Unable to start the network worker.',0>
    ret
endp

proc begin_operation, kind
    mov [reqKind],ecx
    fastcall allowed,operationText
    test eax,eax
    jz .unsupported
    cmp [boverflow],0
    jne .tooLong
    invoke lstrcpyW,reqPath,operationPrefix
    invoke lstrcatW,reqPath,operationText
    mov [reqMethod],postMethod
    fastcall begin_request
    ret
.unsupported:
    fastcall status,unsupportedText
    ret
.tooLong:
    fastcall status,<'The request is too long. Shorten it before sending.',0>
    ret
endp

proc pair
    invoke SendMessageW,[h_server],WM_GETTEXT,2048,serverText
    invoke SendMessageW,[h_token],WM_GETTEXT,128,tokenText
    fastcall validate_connection
    test rax,rax
    jnz .error
    mov [reqMethod],getMethod
    mov [reqKind],1
    invoke lstrcpyW,reqPath,basePathSlash
    fastcall begin_request
    ret
.error:
    fastcall status,rax
    ret
endp

proc forget_connection
if ~ defined TEST_MODE
    invoke CredDeleteW,credentialTarget,1,0
end if
    invoke RtlZeroMemory,tokenText,256
    invoke RtlZeroMemory,sessionText,512
    invoke RtlZeroMemory,repoText,1024
    invoke RtlZeroMemory,responseText,8388608
    invoke RtlZeroMemory,responseBytes,4194304
    invoke RtlZeroMemory,rowJson,16777216
    invoke RtlZeroMemory,requestText,1048576
    invoke RtlZeroMemory,requestBytes,1048576
    invoke RtlZeroMemory,composeText,262144
    invoke RtlZeroMemory,draftText,262144
    invoke RtlZeroMemory,detailText,524288
    invoke RtlZeroMemory,fieldText,65536
    invoke RtlZeroMemory,tempText,131072
    invoke SetWindowTextW,[h_token],emptyText
    invoke SetWindowTextW,[h_compose],emptyText
    invoke SetWindowTextW,[h_detail],emptyText
    invoke SendMessageW,[h_projects],LB_RESETCONTENT,0,0
    invoke SendMessageW,[h_rows],LB_RESETCONTENT,0,0
    invoke SendMessageW,[h_action],CB_RESETCONTENT,0,0
    mov [paired],0
    mov [navPane],0
    mov [advancedMode],0
    mov [canManage],0
    mov [operationCount],0
    mov [rowCount],0
    mov [projectCount],0
    mov [selectedRow],-1
    mov [pollPaused],0
    mov [uncertainWrite],0
    fastcall show_state
    fastcall status,text_status
    ret
endp

proc get_projects
    invoke lstrcpyW,operationText,opProjects
    invoke lstrcpyW,requestText,emptyObject
    mov [boverflow],0
    fastcall begin_operation,3
    ret
endp

proc repo_arguments
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    fastcall bchar,125
    ret
endp

proc session_arguments
    fastcall bbegin,requestText,524288
    fastcall btext,sessionPrefix
    fastcall bquote,sessionText
    fastcall bchar,125
    ret
endp

proc load_list
    cmp [viewMode],3
    je .api
    cmp word [repoText],0
    je .missing
    fastcall repo_arguments
    mov rdx,opSessions
    cmp [viewMode],0
    je .request
    mov rdx,opPulls
.request:
    invoke lstrcpyW,operationText,rdx
    fastcall begin_operation,4
    ret
.missing:
    fastcall status,selectProjectText
    ret
.api:
    fastcall enter_advanced
    invoke SetWindowTextW,[h_detail],apiHelp
    ret
endp

proc load_session
    cmp word [sessionText],0
    je .done
    fastcall session_arguments
    invoke lstrcpyW,operationText,opSession
    fastcall begin_operation,5
.done:
    ret
endp

proc accept_discovery uses rbx
    fastcall json_get,1,keyVersion
    fastcall json_text,rax,tempText,65536
    invoke lstrcmpW,tempText,oneText
    test eax,eax
    jnz .incompatible
    fastcall json_get,1,keyDevice
    test eax,eax
    jz .incompatible
    fastcall json_get,rax,keyPermission
    test eax,eax
    jz .incompatible
    fastcall json_text,rax,tempText,65536
    mov [canManage],0
    invoke lstrcmpW,tempText,manageText
    test eax,eax
    jnz .readonly
    mov [canManage],1
.readonly:
    mov [reqMethod],getMethod
    mov [reqKind],2
    invoke lstrcpyW,reqPath,operationsPath
    fastcall begin_request
    ret
.incompatible:
    fastcall status,<'This server does not expose a compatible mobile API v1 device.',0>
    ret
endp

proc accept_catalog uses rbx rsi rdi r12
    fastcall json_get,1,keyOperations
    test eax,eax
    jz .bad
    mov ebx,eax
    shl rax,5
    cmp dword [jtokens+rax+12],2
    jne .bad
    mov edi,dword [jtokens+rax+16]
    inc ebx
    mov [operationCount],0
    invoke SendMessageW,[h_action],CB_RESETCONTENT,0,0
.loop:
    cmp ebx,edi
    jae .done
    cmp [operationCount],255
    jae .done
    fastcall json_get,rbx,keyName
    test eax,eax
    jz .next
    fastcall json_text,rax,tempText,128
    ; Never permit operation names to inject a path or query.
    mov rsi,tempText
    cmp word [rsi],'a'
    jb .next
    cmp word [rsi],'z'
    ja .next
.validate:
    lodsw
    test ax,ax
    jz .valid
    cmp ax,'a'
    jb .extra
    cmp ax,'z'
    jbe .validate
.extra:
    cmp ax,'_'
    je .validate
    cmp ax,'0'
    jb .next
    cmp ax,'9'
    ja .next
    jmp .validate
.valid:
    mov r12d,[operationCount]
    fastcall json_get,rbx,keyReadOnly
    test eax,eax
    jz .next
    fastcall json_text,rax,fieldText,32768
    invoke lstrcmpW,fieldText,trueText
    mov dword [operationReadOnly+r12*4],0
    test eax,eax
    jnz .write
    mov dword [operationReadOnly+r12*4],1
    jmp .add
.write:
    cmp [canManage],0
    je .next
.add:
    mov rcx,r12
    shl rcx,8
    add rcx,operationNames
    invoke lstrcpyW,rcx,tempText
    invoke SendMessageW,[h_action],CB_ADDSTRING,0,tempText
    inc [operationCount]
.next:
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .loop
.done:
    mov [paired],1
    invoke SetWindowTextW,[h_token],emptyText
    fastcall save_connection
    fastcall show_state
    fastcall get_projects
    ret
.bad:
    fastcall status,jsonError
    ret
endp

proc accept_projects uses rbx rsi rdi r12
    fastcall json_get,1,keyProjects
    test eax,eax
    jz .bad
    mov ebx,eax
    shl rax,5
    cmp dword [jtokens+rax+12],2
    jne .bad
    mov edi,dword [jtokens+rax+16]
    inc ebx
    mov [projectCount],0
    invoke SendMessageW,[h_projects],LB_RESETCONTENT,0,0
.loop:
    cmp ebx,edi
    jae .done
    cmp [projectCount],256
    jae .done
    mov r12d,[projectCount]
    shl r12,10
    add r12,projectRepos
    fastcall json_get,rbx,keyRepo
    test eax,eax
    jz .next
    fastcall json_text,rax,r12,512
    fastcall json_get,rbx,keyLabel
    fastcall json_text,rax,tempText,65536
    cmp word [tempText],0
    jne .label
    invoke lstrcpyW,tempText,r12
.label:
    invoke SendMessageW,[h_projects],LB_ADDSTRING,0,tempText
    inc [projectCount]
.next:
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .loop
.done:
    mov rcx,readyText
    cmp [canManage],0
    jne .save
    mov rcx,readOnlyText
.save:
    cmp [credentialSaved],0
    jne .status
    mov rcx,unsavedText
.status:
    fastcall status,rcx
    ret
.bad:
    fastcall status,jsonError
    ret
endp

proc accept_list uses rbx rsi rdi r12 r13
    mov rdx,keySessions
    cmp [viewMode],0
    je .key
    mov rdx,keyPulls
    cmp [viewMode],1
    je .key
    mov rdx,keyIssues
.key:
    fastcall json_get,1,rdx
    test eax,eax
    jz .bad
    mov ebx,eax
    shl rax,5
    cmp dword [jtokens+rax+12],2
    jne .bad
    mov edi,dword [jtokens+rax+16]
    inc ebx
    mov [rowCount],0
.loop:
    cmp ebx,edi
    jae .done
    cmp [rowCount],512
    jae .done
    mov r12d,[rowCount]
    mov rdx,keyId
    cmp [viewMode],0
    je .id
    mov rdx,keyNumber
.id:
    fastcall json_get,rbx,rdx
    test eax,eax
    jz .next
    mov rdx,r12
    shl rdx,9
    add rdx,rowIds
    fastcall json_text,rax,rdx,256
    fastcall json_get,rbx,keyTitle
    fastcall json_text,rax,fieldText,32768
    mov rcx,r12
    shl rcx,10
    add rcx,rowTitles
    fastcall bbegin,rcx,512
    fastcall btext,fieldText
    cmp [viewMode],0
    jne .rowSnapshot
    fastcall json_get,rbx,keyStatus
    fastcall json_text,rax,fieldText,32768
    cmp word [fieldText],0
    je .rowSnapshot
    fastcall btext,separator
    fastcall btext,fieldText
.rowSnapshot:
    ; Store raw row JSON for board details and filtering. Truncation is visible.
    mov r13,rbx
    shl r13,5
    mov rdx,qword [jtokens+r13]
    mov r8d,dword [jtokens+r13+8]
    inc r8d
    cmp r8d,16384
    jbe .copy
    mov r8d,16384
.copy:
    mov rcx,r12
    shl rcx,15
    add rcx,rowJson
    invoke lstrcpynW,rcx,rdx,r8
    inc [rowCount]
.next:
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .loop
.done:
    fastcall filter_rows
    ret
.bad:
    fastcall status,jsonError
    ret
endp

proc filter_rows uses rbx rsi rdi
    invoke SendMessageW,[h_search],WM_GETTEXT,256,searchText
    invoke SendMessageW,[h_rows],LB_RESETCONTENT,0,0
    mov [selectedRow],-1
    xor ebx,ebx
.loop:
    cmp ebx,[rowCount]
    jae .done
    cmp word [searchText],0
    je .add
    mov rcx,rbx
    shl rcx,15
    add rcx,rowJson
    invoke StrStrIW,rcx,searchText
    test rax,rax
    jz .next
.add:
    mov rsi,rbx
    shl rsi,10
    add rsi,rowTitles
    cmp word [rsi],0
    jne .label
    mov rsi,newTitle
.label:
    invoke SendMessageW,[h_rows],LB_ADDSTRING,0,rsi
    test eax,eax
    js .next
    mov [rowMap+rax*4],ebx
    mov edi,eax
    cmp [viewMode],0
    jne .next
    mov rcx,rbx
    shl rcx,9
    add rcx,rowIds
    invoke lstrcmpW,rcx,sessionText
    test eax,eax
    jnz .next
    invoke SendMessageW,[h_rows],LB_SETCURSEL,rdi,0
    mov [selectedRow],ebx
.next:
    inc ebx
    jmp .loop
.done:
    cmp [rowCount],0
    jne .return
    invoke SetWindowTextW,[h_detail],noRowsText
.return:
    ret
endp

proc select_row uses rbx rsi
    invoke SendMessageW,[h_rows],LB_GETCURSEL,0,0
    cmp eax,-1
    je .done
    mov ebx,dword [rowMap+rax*4]
    mov [selectedRow],ebx
    mov rcx,rbx
    shl rcx,10
    add rcx,rowTitles
    mov rdx,rcx
    invoke SetWindowTextW,[h_detailTitle],rdx
    mov rsi,rbx
    shl rsi,9
    add rsi,rowIds
    cmp [viewMode],0
    jne .board
    invoke lstrcpyW,sessionText,rsi
    fastcall load_session
    ret
.board:
    invoke lstrcpyW,numberText,rsi
    mov word [sessionText],0
    cmp [viewMode],2
    je .issue
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    fastcall btext,prPrefix
    fastcall btext,numberText
    fastcall bchar,125
    invoke lstrcpyW,operationText,opPull
    fastcall begin_operation,5
    ret
.issue:
    mov rcx,rbx
    shl rcx,15
    add rcx,rowJson
    fastcall json_parse,rcx
    test eax,eax
    jz .truncated
    fastcall render_result
    ret
.truncated:
    invoke SetWindowTextW,[h_detail],<'This issue is too large for the list snapshot. Use API tools or open the dashboard for the full issue.',0>
.done:
    ret
endp

; Human-readable event stream plus a formatted JSON view for extensible board data.
proc render_result uses rbx rsi rdi r12
    fastcall bbegin,detailText,262144
    fastcall json_get,1,keyEvents
    test eax,eax
    jz .files
    mov ebx,eax
    shl rax,5
    mov edi,dword [jtokens+rax+16]
    inc ebx
.event:
    cmp ebx,edi
    jae .display
    fastcall json_get,rbx,keyKind
    fastcall json_text,rax,fieldText,32768
    invoke lstrcmpW,fieldText,setupText
    test eax,eax
    jz .nextEvent
    invoke lstrcmpW,fieldText,statusText
    test eax,eax
    jz .nextEvent
    fastcall btext,fieldText
    fastcall btext,separator
    fastcall json_get,rbx,keyTime
    fastcall json_text,rax,fieldText,32768
    fastcall btext,fieldText
    fastcall btext,newline
    fastcall json_get,rbx,keyText
    test eax,eax
    jnz .eventText
    fastcall json_get,rbx,keySummary
    test eax,eax
    jnz .eventText
    fastcall json_get,rbx,keyQuestion
.eventText:
    fastcall json_text,rax,fieldText,32768
    fastcall btext,fieldText
    fastcall btext,doubleNewline
.nextEvent:
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .event
.files:
    fastcall json_get,1,keyFiles
    test eax,eax
    jz .body
    mov ebx,eax
    shl rax,5
    mov edi,dword [jtokens+rax+16]
    inc ebx
.file:
    cmp ebx,edi
    jae .page
    fastcall json_get,rbx,keyFilename
    fastcall json_text,rax,fieldText,32768
    fastcall btext,fieldText
    fastcall btext,doubleNewline
    fastcall json_get,rbx,keyPatch
    fastcall json_text,rax,fieldText,32768
    fastcall btext,fieldText
    fastcall btext,doubleNewline
    shl rbx,5
    mov ebx,dword [jtokens+rbx+16]
    jmp .file
.page:
    fastcall btext,<'More files may be available. Check nextPage using the response below.',13,10,13,10,0>
    jmp .pretty
.body:
    mov ebx,1
    fastcall json_get,1,keyPull
    test eax,eax
    jz .bodyKey
    mov ebx,eax
.bodyKey:
    fastcall json_get,rbx,keyBody
    test eax,eax
    jz .pretty
    fastcall json_text,rax,fieldText,32768
    fastcall btext,fieldText
    fastcall btext,doubleNewline
.pretty:
    ; Pretty print the parsed root without changing string contents.
    mov rsi,qword [jtokens+32]
    xor ebx,ebx
    xor edi,edi
    xor r12d,r12d
.char:
    lodsw
    test ax,ax
    jz .display
    movzx ecx,ax
    test edi,edi
    jz .outside
    fastcall bchar,rcx
    test r12d,r12d
    jnz .unescape
    cmp word [rsi-2],92
    je .escape
    cmp word [rsi-2],'"'
    jne .char
    xor edi,edi
    jmp .char
.escape:
    mov r12d,1
    jmp .char
.unescape:
    xor r12d,r12d
    jmp .char
.outside:
    cmp cx,'"'
    je .quote
    cmp cx,'{'
    je .open
    cmp cx,'['
    je .open
    cmp cx,'}'
    je .close
    cmp cx,']'
    je .close
    cmp cx,','
    je .comma
    cmp cx,':'
    je .colon
    cmp cx,32
    jbe .char
    fastcall bchar,rcx
    jmp .char
.quote:
    mov edi,1
    fastcall bchar,rcx
    jmp .char
.open:
    fastcall bchar,rcx
    inc ebx
    jmp .line
.close:
    dec ebx
    fastcall btext,newline
    fastcall indent,rbx
    movzx ecx,word [rsi-2]
    fastcall bchar,rcx
    jmp .char
.comma:
    fastcall bchar,44
.line:
    fastcall btext,newline
    fastcall indent,rbx
    jmp .char
.colon:
    fastcall btext,colon
    jmp .char
.display:
    cmp [boverflow],0
    je .setDetail
    invoke lstrcpyW,detailText+(262144-96)*2,<'[Display limit reached. Narrow the request or open the web dashboard.]',0>
    fastcall status,<'This response is larger than the display limit. The visible text is truncated.',0>
.setDetail:
    invoke SetWindowTextW,[h_detail],detailText
    ret
endp

proc indent uses rbx, count
    mov ebx,ecx
    shl ebx,1
.loop:
    test ebx,ebx
    jz .done
    fastcall bchar,32
    dec ebx
    jmp .loop
.done:
    ret
endp

proc complete_request uses rbx
    mov [busy],0
    fastcall update_enabled
    cmp [netError],0
    jne .error
    fastcall json_parse,responseText
    test eax,eax
    jz .invalid
    fastcall status,loadedText
    mov eax,[reqKind]
    cmp eax,1
    je .discovery
    cmp eax,2
    je .catalog
    cmp eax,3
    je .projects
    cmp eax,4
    je .list
    cmp eax,6
    je .mutation
    cmp eax,7
    je .revoked
    fastcall render_result
    ret
.discovery:
    fastcall accept_discovery
    ret
.catalog:
    fastcall accept_catalog
    ret
.projects:
    fastcall accept_projects
    ret
.list:
    fastcall accept_list
    cmp [viewMode],0
    jne .listDone
    fastcall load_session
.listDone:
    ret
.mutation:
    invoke SetWindowTextW,[h_compose],emptyText
    fastcall json_get,1,keySession
    test eax,eax
    jz .refresh
    fastcall json_get,rax,keyId
    test eax,eax
    jz .refresh
    fastcall json_text,rax,sessionText,256
.refresh:
    mov [viewMode],0
    invoke SendMessageW,[h_view],CB_SETCURSEL,0,0
    fastcall load_list
    ret
.revoked:
    fastcall forget_connection
    fastcall status,expiredText
    ret
.invalid:
    mov [pollPaused],1
    fastcall status,jsonError
    ret
.error:
    mov [pollPaused],1
    cmp [reqKind],6
    je .uncertain
    cmp [reqKind],8
    jne .check401
    fastcall allowed,operationText
    cmp eax,2
    jne .check401
.uncertain:
    mov [uncertainWrite],1
    fastcall update_enabled
.check401:
    cmp [httpStatus],401
    je .revoked
    ; Error payloads may contain useful server explanations, never credentials.
    fastcall json_parse,responseText
    test eax,eax
    jz .errorStatus
    fastcall json_get,1,keyError
    test eax,eax
    jz .errorStatus
    fastcall json_text,rax,tempText,65536
    invoke SetWindowTextW,[h_detail],tempText
.errorStatus:
    fastcall status,errorText
    ret
endp

proc send_message uses rbx, startNew
    mov ebx,ecx
    cmp [canManage],0
    je .unsupported
    invoke SendMessageW,[h_compose],WM_GETTEXT,131072,composeText
    test eax,eax
    jz .empty
    test ebx,ebx
    jnz .new
    cmp word [sessionText],0
    je .noSession
    fastcall bbegin,requestText,524288
    fastcall btext,sessionPrefix
    fastcall bquote,sessionText
    fastcall btext,messagePrefix
    jmp .body
.new:
    cmp word [repoText],0
    je .noProject
    fastcall bbegin,requestText,524288
    fastcall btext,repoPrefix
    fastcall bquote,repoText
    fastcall btext,promptPrefix
.body:
    fastcall bquote,composeText
    fastcall bchar,125
    mov rdx,opMessage
    test ebx,ebx
    jz .request
    mov rdx,opStart
.request:
    invoke lstrcpyW,operationText,rdx
    fastcall begin_operation,6
    ret
.noSession:
    fastcall status,selectSessionText
    ret
.noProject:
    fastcall status,selectProjectText
    ret
.empty:
    fastcall status,newPrompt
    ret
.unsupported:
    fastcall status,unsupportedText
    ret
endp

proc enter_advanced
    mov [navPane],2
    cmp [advancedMode],0
    jne .show
    invoke SendMessageW,[h_compose],WM_GETTEXT,131072,draftText
.show:
    mov [advancedMode],1
    fastcall show_state
    invoke SendMessageW,[h_action],CB_SETCURSEL,0,0
    fastcall prepare_action
    ret
endp

proc prepare_action uses rbx
    invoke SendMessageW,[h_action],CB_GETCURSEL,0,0
    cmp eax,-1
    je .done
    mov ebx,eax
    shl rbx,8
    add rbx,operationNames
    invoke lstrcpyW,operationText,rbx
    ; Templates contain exact API argument names from the cloned Swift client.
    fastcall action_template
    invoke SetWindowTextW,[h_compose],requestText
.done:
    ret
endp

proc execute_action
    invoke SendMessageW,[h_compose],WM_GETTEXT,524288,requestText
    fastcall json_parse,requestText
    test eax,eax
    jz .invalid
    cmp dword [jtokens+32+12],1
    jne .invalid
    invoke SendMessageW,[h_action],CB_GETCURSEL,0,0
    cmp eax,-1
    je .done
    shl rax,8
    add rax,operationNames
    invoke lstrcpyW,operationText,rax
    fastcall allowed,operationText
    test eax,eax
    jz .unsupported
    cmp eax,2
    jne .send
    cmp [uncertainWrite],0
    jne .uncertain
    invoke MessageBoxW,[mainWindow],writeConfirm,operationText,MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2
    cmp eax,IDYES
    jne .done
.send:
    mov [boverflow],0
    fastcall begin_operation,8
.done:
    ret
.unsupported:
    fastcall status,unsupportedText
    ret
.invalid:
    fastcall status,invalidJsonText
    ret
.uncertain:
    fastcall status,<'The previous write has an unknown outcome. Refresh and verify the server state before executing another write.',0>
    ret
endp

proc window_proc uses rbx rsi rdi, hwnd,message,wparam,lparam
    mov [hwnd],rcx
    mov [message],rdx
    mov [wparam],r8
    mov [lparam],r9
    cmp edx,WM_DESTROY
    je .destroy
    cmp edx,WM_SIZE
    je .resize
    cmp edx,WM_GETMINMAXINFO
    je .minimum
    cmp edx,WM_COMMAND
    je .command
    cmp edx,WM_APP+1
    je .complete
    cmp edx,WM_TIMER
    je .timer
    cmp edx,WM_ACTIVATE
    je .activate
    cmp edx,WM_CTLCOLORSTATIC
    je .static
    cmp edx,WM_CTLCOLOREDIT
    je .edit
    cmp edx,WM_CTLCOLORLISTBOX
    je .edit
    invoke DefWindowProcW,rcx,rdx,r8,r9
    ret
.minimum:
    mov dword [r9+24],480
    mov dword [r9+28],680
    xor eax,eax
    ret
.static:
    mov rax,[lparam]
    cmp rax,[h_view]
    je .edit
    cmp rax,[h_action]
    je .edit
    cmp rax,[h_detail]
    je .edit
    cmp rax,[h_compose]
    je .edit
    invoke SetBkMode,r8,OPAQUE
    invoke SetBkColor,[wparam],0F5F9FAh
    mov rax,[lparam]
    cmp rax,[h_brand]
    je .brand
    invoke SetTextColor,[wparam],0252526h
    mov rax,[backgroundBrush]
    ret
.brand:
    invoke SetTextColor,[wparam],04264C9h
    mov rax,[backgroundBrush]
    ret
.edit:
    invoke SetBkMode,[wparam],OPAQUE
    invoke SetBkColor,[wparam],0FFFFFFh
    invoke SetTextColor,[wparam],0252526h
    mov rax,[surfaceBrush]
    ret
.resize:
    fastcall layout
    jmp .zero
.complete:
    fastcall complete_request
    jmp .zero
.activate:
    and r8d,0ffffh
    mov [activeWindow],r8d
    jmp .zero
.timer:
    cmp [pollPaused],0
    jne .zero
    cmp [busy],0
    jne .zero
    cmp [paired],0
    je .zero
    cmp [activeWindow],0
    je .zero
    cmp [advancedMode],0
    jne .zero
    cmp [viewMode],0
    jne .zero
    fastcall load_session
    jmp .zero
.command:
    mov ebx,r8d
    shr ebx,16
    and r8d,0ffffh
    cmp r8d,204
    je .search
    ; Pane navigation changes visibility only; keep it available during requests.
    cmp r8d,215
    je .navProjects
    cmp r8d,216
    je .navList
    cmp r8d,217
    je .navDetail
    cmp [busy],0
    jne .zero
    cmp r8d,102
    je .pair
    cmp r8d,200
    je .project
    cmp r8d,201
    je .refresh
    cmp r8d,202
    je .forget
    cmp r8d,203
    je .view
    cmp r8d,205
    je .row
    cmp r8d,208
    je .send
    cmp r8d,209
    je .new
    cmp r8d,210
    je .advanced
    cmp r8d,211
    je .action
    cmp r8d,212
    je .execute
    cmp r8d,213
    je .back
    cmp r8d,214
    je .revoke
    jmp .zero
.pair:
    fastcall pair
    jmp .zero
.project:
    cmp ebx,LBN_SELCHANGE
    jne .zero
    invoke SendMessageW,[h_projects],LB_GETCURSEL,0,0
    cmp eax,-1
    je .zero
    shl rax,10
    add rax,projectRepos
    invoke lstrcpyW,repoText,rax
    mov word [sessionText],0
    mov word [numberText],0
    invoke SetWindowTextW,[h_detail],emptyText
    mov [navPane],1
    fastcall layout
    fastcall load_list
    jmp .zero
.refresh:
    mov [pollPaused],0
    mov [uncertainWrite],0
    fastcall update_enabled
    cmp word [repoText],0
    je .refreshProjects
    fastcall load_list
    jmp .zero
.refreshProjects:
    fastcall get_projects
    jmp .zero
.forget:
    invoke MessageBoxW,[mainWindow],forgetConfirm,windowTitle,MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2
    cmp eax,IDYES
    jne .zero
    fastcall forget_connection
    jmp .zero
.view:
    cmp ebx,CBN_SELCHANGE
    jne .zero
    invoke SendMessageW,[h_view],CB_GETCURSEL,0,0
    mov [viewMode],eax
    invoke InvalidateRect,[h_view],0,TRUE
    invoke UpdateWindow,[h_view]
    mov word [sessionText],0
    mov word [numberText],0
    mov [advancedMode],0
    mov [rowCount],0
    invoke SendMessageW,[h_rows],LB_RESETCONTENT,0,0
    invoke SetWindowTextW,[h_detailTitle],clearTitle
    fastcall show_state
    fastcall load_list
    jmp .zero
.row:
    cmp ebx,LBN_SELCHANGE
    jne .zero
    fastcall select_row
    mov [navPane],2
    fastcall layout
    jmp .zero
.search:
    cmp ebx,EN_CHANGE
    jne .zero
    fastcall filter_rows
    jmp .zero
.send:
    fastcall send_message,0
    jmp .zero
.new:
    fastcall send_message,1
    jmp .zero
.advanced:
    fastcall enter_advanced
    jmp .zero
.action:
    cmp ebx,CBN_SELCHANGE
    jne .zero
    fastcall prepare_action
    jmp .zero
.execute:
    fastcall execute_action
    jmp .zero
.back:
    mov [advancedMode],0
    invoke SetWindowTextW,[h_compose],draftText
    fastcall show_state
    jmp .zero
.navProjects:
    mov [navPane],0
    fastcall layout
    jmp .zero
.navList:
    mov [navPane],1
    fastcall layout
    jmp .zero
.navDetail:
    mov [navPane],2
    fastcall layout
    jmp .zero
.revoke:
    invoke MessageBoxW,[mainWindow],revokeConfirm,windowTitle,MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2
    cmp eax,IDYES
    jne .zero
    mov [reqMethod],deleteMethod
    mov [reqKind],7
    invoke lstrcpyW,reqPath,tokenPath
    fastcall begin_request
    jmp .zero
.destroy:
    invoke KillTimer,[hwnd],1
    invoke RtlZeroMemory,tokenText,256
    invoke PostQuitMessage,0
.zero:
    xor eax,eax
    ret
endp

include 'templates.inc'
