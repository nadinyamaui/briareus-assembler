format PE64 console 6.0
entry start
include 'win64wx.inc'
section '.text' code readable executable
start:
    sub rsp,8
    mov ebx,1
    fastcall json_parse,sample
    test eax,eax
    jz fail
    fastcall json_get,1,keyNested
    fastcall json_get,rax,keyMessage
    fastcall json_text,rax,output,256
    invoke lstrcmpW,output,expected
    test eax,eax
    jnz fail
    inc ebx
    fastcall json_get,1,keyMissing
    test eax,eax
    jnz fail
    inc ebx
    mov rsi,invalidCases
.invalid:
    mov rcx,[rsi]
    test rcx,rcx
    jz .builder
    fastcall json_parse,rcx
    test eax,eax
    jnz fail
    inc ebx
    add rsi,8
    jmp .invalid
.builder:
    fastcall bbegin,output,256
    fastcall bquote,expected
    fastcall json_parse,output
    test eax,eax
    jz fail
    inc ebx
    fastcall json_text,1,roundtrip,256
    invoke lstrcmpW,roundtrip,expected
    test eax,eax
    jnz fail
    inc ebx
    fastcall bbegin,output,4
    fastcall bquote,expected
    cmp [boverflow],1
    jne fail
    inc ebx
    cmp word [output+6],0
    jne fail
    ; Non-string nested objects, exponent, booleans and empty containers.
    fastcall json_parse,validNumbers
    test eax,eax
    jz fail
    invoke ExitProcess,0
fail:
    invoke ExitProcess,rbx
include '../src/json.inc'
section '.data' data readable writeable
sample du '{"nested":{"message":"Espa\u00f1a\n\"hello\" \\ \ud83d\ude80"},"arr":[null,true,false,1.5e-2]}',0
expected du 'Espa',0f1h,'a',10,'"hello" \ ',0d83dh,0de80h,0
keyNested du 'nested',0
keyMessage du 'message',0
keyMissing du 'missing',0
validNumbers du ' [ {}, [], -0, 0.12, 1e10, -15.25E+10, null, false, true ] ',0
invalidCases dq invalid1,invalid2,invalid3,invalid4,invalid5,invalid6,invalid7,invalid8,invalid9,invalid10,invalid11,invalid12,0
invalid1 du '{"a":}',0
invalid2 du '[1,]',0
invalid3 du '{"a":1,}',0
invalid4 du '01',0
invalid5 du '1.',0
invalid6 du '1e',0
invalid7 du 'true false',0
invalid8 du '"bad\x"',0
invalid9 du '"bad\u0xx0"',0
invalid10 du '{a:1}',0
invalid11 du '"unclosed',0
invalid12 du '-',0
include '../src/json-data.inc'
output rw 256
roundtrip rw 256
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL'
include 'api/kernel32.inc'
section '.reloc' fixups data readable discardable
