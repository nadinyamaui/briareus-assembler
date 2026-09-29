format PE64 GUI 6.0
entry start
include 'win64wx.inc'
EM_SETCUEBANNER = 1501h
WM_APP = 8000h

section '.text' code readable executable
start:
    sub rsp,8
    ; DPI awareness is declared in the application manifest.
    invoke GetModuleHandleW,0
    mov [instance],rax
    mov [wc.hInstance],rax
    invoke CreateSolidBrush,0F5F9FAh
    mov [backgroundBrush],rax
    mov [wc.hbrBackground],rax
    invoke CreateSolidBrush,0FFFFFFh
    mov [surfaceBrush],rax
    invoke LoadCursorW,0,IDC_ARROW
    mov [wc.hCursor],rax
    invoke LoadIconW,0,IDI_APPLICATION
    mov [wc.hIcon],rax
    mov [wc.hIconSm],rax
    invoke CreateFontW,-17,0,0,0,400,0,0,0,1,0,0,5,0,<'Segoe UI',0>
    mov [fontBody],rax
    invoke CreateFontW,-32,0,0,0,400,0,0,0,1,0,0,5,0,<'Georgia',0>
    mov [fontTitle],rax
    invoke CreateFontW,-15,0,0,0,400,0,0,0,1,0,0,5,0,<'Consolas',0>
    mov [fontMono],rax
    invoke RegisterClassExW,wc
    invoke CreateWindowExW,WS_EX_CONTROLPARENT,className,windowTitle,WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN,100,60,1200,820,0,0,[instance],0
    test rax,rax
    jz .exit
    mov [mainWindow],rax
    fastcall create_ui
    invoke ShowWindow,[mainWindow],SW_SHOWNORMAL
    invoke UpdateWindow,[mainWindow]
.loop:
    invoke GetMessageW,msg,0,0,0
    cmp eax,1
    jne .exit
    invoke IsDialogMessageW,[mainWindow],msg
    test eax,eax
    jnz .loop
    invoke TranslateMessage,msg
    invoke DispatchMessageW,msg
    jmp .loop
.exit:
    invoke ExitProcess,0

include 'json.inc'
include 'net.inc'
include 'ui.inc'

section '.data' data readable writeable
className du 'BriareusAssemblerWindow',0
windowTitle du 'Briareus',0
wc WNDCLASSEX sizeof.WNDCLASSEX,CS_HREDRAW+CS_VREDRAW,window_proc,0,0,0,0,0,0,0,className,0
msg MSG
clientRect RECT
instance dq 0
mainWindow dq 0
fontBody dq 0
fontTitle dq 0
fontMono dq 0
backgroundBrush dq 0
surfaceBrush dq 0
paired dd 0
busy dd 0
advancedMode dd 0
viewMode dd 0
navPane dd 0
layoutMode dd 0
activeWindow dd 1
canManage dd 0
pollPaused dd 0
uncertainWrite dd 0
projectCount dd 0
rowCount dd 0
operationCount dd 0
selectedRow dd -1
reqKind dd 0
reqMethod dq getMethod
netError dd 0
httpStatus dd 0
bytesRead dd 0
querySize dd 0
disabledFeatures dd 7
highAutologon dd 2
credentialSaved dd 0
credentialPtr dq 0
credentialTokenPtr dq 0
credentialTarget du 'BriareusAssembler/connection',0
credentialUser du 'Briareus device',0
getMethod du 'GET',0
postMethod du 'POST',0
deleteMethod du 'DELETE',0
basePath du '/api/mobile/v1',0
basePathSlash du '/api/mobile/v1/',0
operationsPath du '/api/mobile/v1/operations',0
operationPrefix du '/api/mobile/v1/operations/',0
tokenPath du '/api/mobile/v1/token',0
slashText du '/',0
agentText du 'Briareus-Assembler/0.1 Windows',0
jsonMime du 'application/json',0
headerPrefix du 'Authorization: Bearer ',0
headerSuffix du 13,10,'Accept: application/json',13,10,'Content-Type: application/json',13,10,0
addressError du 'Use an HTTPS server origin, without a username, query or fragment.',0
tokenError du 'A device token must be brm_ followed by 43 letters, digits, hyphens or underscores.',0
redirectError du 'The server redirected the request. Check the mobile API proxy settings.',0
jsonError du 'The server did not return valid JSON. Check the address and proxy settings.',0
oversizeError du 'The response exceeds the 4 MB limit. Narrow the request.',0
networkErrorFormat du 'Connection failed (Windows error %u). A write may have completed; refresh before trying again.',0
httpErrorFormat du 'Server returned HTTP %u. Refresh before repeating a write.',0
serverHint du 'https://briareus.example.com',0
tokenHint du 'brm_...',0
searchHint du 'Search titles, authors, labels...',0
emptyText du 0
emptyObject du '{}',0
busyText du 'Contacting your server...',0
readyText du 'Connected. Select a project or conversation.',0
readOnlyText du 'Connected with read-only access. Changes are disabled.',0
readComposerLabel du 'Read-only access. Use Actions / API for read operations.',0
boardComposerLabel du 'Task for a new conversation',0
unsavedText du 'Connected, but Windows could not save the credential. It will not be restored next time.',0
loadedText du 'Up to date.',0
unsupportedText du 'This operation is unavailable for this device or server.',0
selectProjectText du 'Select a project first.',0
selectSessionText du 'Select a conversation first.',0
invalidJsonText du 'Arguments must be a valid JSON object. Nothing was sent.',0
writeConfirm du 'Execute this operation with the arguments shown? It may start a paid agent or change GitHub. Requests are sent once.',0
forgetConfirm du 'Forget this connection on this PC? Running agents and the server token will remain active.',0
revokeConfirm du 'Revoke this device token on the server? This connection will stop working.',0
newPrompt du 'Write the task in the message box, then choose New conversation. Uses the project default runtime; use Actions / API to choose a branch or runtime.',0
newTitle du 'New conversation',0
noRowsText du 'No matching items. Refresh or change the search.',0
clearTitle du 'Select an item',0
apiHelp du 'Select a server operation. Edit the JSON arguments below, then Execute. Only operations allowed by the server and device permissions are listed.',0
confirmTitle du 'Confirm operation',0
keyVersion du 'version',0
keyDevice du 'device',0
keyPermission du 'permission',0
manageText du 'manage',0
keyOperations du 'operations',0
keyName du 'name',0
keyReadOnly du 'readOnly',0
trueText du 'true',0
oneText du '1',0
keyProjects du 'projects',0
keyRepo du 'repo',0
keyLabel du 'label',0
keySessions du 'sessions',0
keySession du 'session',0
keyId du 'id',0
keyTitle du 'title',0
keyStatus du 'status',0
keyPulls du 'pulls',0
keyIssues du 'issues',0
keyNumber du 'number',0
keyBody du 'body',0
keyEvents du 'events',0
keyKind du 'kind',0
keyText du 'text',0
keySummary du 'summary',0
keyQuestion du 'question',0
keyTime du 't',0
keyFiles du 'files',0
keyFilename du 'filename',0
keyPatch du 'patch',0
keyPull du 'pr',0
keyError du 'error',0
keyTranscribe du 'transcribe',0
keyNextPage du 'nextPage',0
setupText du 'setup',0
statusText du 'status',0
opProjects du 'projects',0
opSessions du 'sessions',0
opSession du 'session',0
opPulls du 'pulls',0
opPull du 'pull',0
opMessage du 'message',0
opStart du 'start_session',0
opFiles du 'pull_files',0
opRevoke du 'revoke_device_token',0
repoPrefix du '{"repo":',0
sessionPrefix du '{"sessionId":',0
prPrefix du ',"pr":',0
messagePrefix du ',"text":',0
promptPrefix du ',"prompt":',0
newline du 13,10,0
doubleNewline du 13,10,13,10,0
separator du '  |  ',0
colon du ': ',0
arrowText du ' > ',0
operationInfo du 'Request completed. The result is shown above.',0
expiredText du 'This device token expired or was revoked. Connect with a new token.',0
include 'templates-data.inc'
include 'controls-data.inc'
include 'json-data.inc'
if defined TEST_MODE
include '../tests/fixtures.inc'
end if
align 8
urlParts rb 104
credential rb 80
credentialBlob rb 8192
serverText rw 2048
hostText rw 256
tokenText rw 128
headerText rw 1024
mimeText rw 128
reqPath rw 1024
errorText rw 2048
tempText rw 65536
fieldText rw 32768
searchText rw 256
repoText rw 512
sessionText rw 256
numberText rw 64
operationText rw 128
requestText rw 524288
composeText rw 131072
draftText rw 131072
detailText rw 262144
requestBytes rb 1048576
responseBytes rb 4194304
responseText rw 4194304
projectRepos rw 256*512
rowIds rw 512*256
rowTitles rw 512*512
rowJson rw 512*16384
rowMap rd 512
operationNames rw 256*128
operationReadOnly rd 256

section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',user32,'USER32.DLL',gdi32,'GDI32.DLL',winhttp,'WINHTTP.DLL',advapi32,'ADVAPI32.DLL',shlwapi,'SHLWAPI.DLL',uxtheme,'UXTHEME.DLL'
include 'api/kernel32.inc'
include 'api/user32.inc'
include 'api/gdi32.inc'
import winhttp,WinHttpOpen,'WinHttpOpen',WinHttpConnect,'WinHttpConnect',WinHttpOpenRequest,'WinHttpOpenRequest',WinHttpSendRequest,'WinHttpSendRequest',WinHttpReceiveResponse,'WinHttpReceiveResponse',WinHttpQueryHeaders,'WinHttpQueryHeaders',WinHttpReadData,'WinHttpReadData',WinHttpCloseHandle,'WinHttpCloseHandle',WinHttpSetOption,'WinHttpSetOption',WinHttpSetTimeouts,'WinHttpSetTimeouts',WinHttpCrackUrl,'WinHttpCrackUrl'
import advapi32,CredWriteW,'CredWriteW',CredReadW,'CredReadW',CredFree,'CredFree',CredDeleteW,'CredDeleteW'
import shlwapi,StrStrIW,'StrStrIW'
import uxtheme,SetWindowTheme,'SetWindowTheme'

section '.rsrc' resource data readable
directory RT_MANIFEST,manifests
resource manifests,1,LANG_NEUTRAL,manifest
resdata manifest
file 'app.manifest'
endres

section '.reloc' fixups data readable discardable


