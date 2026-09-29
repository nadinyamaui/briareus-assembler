format PE64 console 6.0
entry start
include 'win64wx.inc'
section '.text' code readable executable
start:
 sub rsp,8
 mov rsi,cases
 mov ebx,1
.loop:
 mov rdx,[rsi]
 test rdx,rdx
 jz .token
 invoke lstrcpyW,serverText,rdx
 fastcall validate_connection
 cmp qword [rsi+8],0
 jne .expectError
 test rax,rax
 jnz fail
 jmp .next
.expectError:
 test rax,rax
 jz fail
.next:
 inc ebx
 add rsi,16
 jmp .loop
.token:
 invoke lstrcpyW,serverText,valid1
 mov word [tokenText+8],'!'
 fastcall validate_connection
 cmp rax,tokenError
 jne fail
 inc ebx
 mov word [tokenText+8],'a'
 mov word [tokenText+92],0
 fastcall validate_connection
 cmp rax,tokenError
 jne fail
good:
 invoke ExitProcess,0
fail:
 invoke ExitProcess,rbx
include '../src/net.inc'
section '.data' data readable writeable
cases dq valid1,0,valid2,0,valid3,0,valid4,0,valid5,0,bad1,1,bad2,1,bad3,1,bad4,1,bad5,1,bad6,1,bad7,1,bad8,1,0,0
valid1 du 'https://example.com',0
valid2 du 'https://example.com/',0
valid3 du 'https://example.com:8443/api/mobile/v1',0
valid4 du 'https://example.com/api/mobile/v1/',0
valid5 du 'https://[::1]:8443',0
bad1 du 'http://example.com',0
bad2 du 'https://user:pass@example.com',0
bad3 du 'https://example.com?key=secret',0
bad4 du 'https://example.com#fragment',0
bad5 du 'https://example.com/another/path',0
bad6 du 'not a url',0
bad7 du 'https://',0
bad8 du 'https://example.com:0',0
tokenText du 'brm_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',0
addressError du 'address',0
tokenError du 'token',0
slashText du '/',0
basePath du '/api/mobile/v1',0
basePathSlash du '/api/mobile/v1/',0
urlParts rb 104
hostText rw 256
serverText rw 2048
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',winhttp,'WINHTTP.DLL'
include 'api/kernel32.inc'
import winhttp,WinHttpCrackUrl,'WinHttpCrackUrl'
