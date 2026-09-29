"""Build-time only: emit repetitive Win32 control declarations. No runtime Python."""
from pathlib import Path

controls = [
 ('brand','STATIC','Briareus',0,20,14,280,44,10),
 ('tagline','STATIC','Your projects. Your agents.',0,22,61,500,24,11),
 ('serverLabel','STATIC','Server address',0,48,145,480,22,12),
 ('server','EDIT','',0x00810080,48,174,570,36,100),
 ('tokenLabel','STATIC','Device token',0,48,230,480,22,13),
 ('token','EDIT','',0x008100a0,48,259,570,36,101),
 ('pairHelp','STATIC','Create a token in the web dashboard: Settings > Mobile devices.\r\nThe connection uses HTTPS. Windows protects your saved token.',0,48,318,800,64,14),
 ('connect','BUTTON','Connect',0x00010001,48,404,180,42,102),
 ('projectsLabel','STATIC','Projects',0,20,106,260,25,20),
 ('projects','LISTBOX','',0x00a10101,20,140,244,390,200),
 ('refresh','BUTTON','Refresh',0x00010000,20,548,116,34,201),
 ('forget','BUTTON','Disconnect',0x00010000,144,548,120,34,202),
 ('revoke','BUTTON','Revoke device token',0x00010000,952,46,208,34,214),
 ('navProjects','BUTTON','Projects',0x00010000,16,124,140,32,215),
 ('navList','BUTTON','List',0x00010000,164,124,140,32,216),
 ('navDetail','BUTTON','Detail',0x00010000,312,124,140,32,217),
 ('view','COMBOBOX','',0x00210003,288,106,270,180,203),
 ('search','EDIT','',0x00810080,288,148,270,32,204),
 ('rows','LISTBOX','',0x00a10101,288,194,270,380,205),
 ('detailTitle','STATIC','Select a conversation',0,586,108,550,28,21),
 ('detail','EDIT','Choose a project to get started.',0x00a10844,586,148,550,340,206),
 ('composeLabel','STATIC','Message',0,586,504,550,22,22),
 ('compose','EDIT','',0x00a11044,586,533,550,100,207),
 ('send','BUTTON','Send message',0x00010000,586,644,142,36,208),
 ('new','BUTTON','New conversation',0x00010000,738,644,164,36,209),
 ('advanced','BUTTON','Actions / API',0x00010000,912,644,140,36,210),
 ('action','COMBOBOX','',0x00210003,586,504,340,400,211),
 ('execute','BUTTON','Execute',0x00010000,586,644,142,36,212),
 ('back','BUTTON','Back to message',0x00010000,740,644,162,36,213),
 ('status','STATIC','Enter your server address and device token.',0,20,700,1120,30,23),
]

out = []
for name, cls, title, style, x,y,w,h,ident in controls:
    out.append(f"    invoke CreateWindowExW,0,class{cls},text_{name},WS_CHILD+WS_VISIBLE+{style:#x},{x},{y},{w},{h},[mainWindow],{ident},[instance],0\n    mov [h_{name}],rax\n    invoke SendMessageW,rax,WM_SETFONT,[fontBody],TRUE")
out += ['    invoke SendMessageW,[h_brand],WM_SETFONT,[fontTitle],TRUE',
        '    invoke SendMessageW,[h_detail],WM_SETFONT,[fontMono],TRUE',
        '    invoke SendMessageW,[h_server],EM_LIMITTEXT,1900,0',
        '    invoke SendMessageW,[h_token],EM_LIMITTEXT,47,0',
        '    invoke SendMessageW,[h_compose],EM_LIMITTEXT,120000,0',
        '    invoke SendMessageW,[h_detail],EM_LIMITTEXT,262143,0',
        '    invoke SendMessageW,[h_search],EM_SETCUEBANNER,0,searchHint',
        '    invoke SendMessageW,[h_server],EM_SETCUEBANNER,0,serverHint',
        '    invoke SendMessageW,[h_token],EM_SETCUEBANNER,0,tokenHint']
for v in ['Conversations','Pull requests','Issues','API tools']:
    out.append(f"    invoke SendMessageW,[h_view],CB_ADDSTRING,0,<'{v}',0>")
out.append('    invoke SendMessageW,[h_view],CB_SETCURSEL,0,0')
Path('src/controls.inc').write_text('\n'.join(out)+'\n')
out=[]
for name,cls,title,style,x,y,w,h,ident in controls:
    out.append(f'h_{name} dq 0')
    values=','.join(str(ord(c)) for c in title)+(',' if title else '')+'0'
    out.append(f'text_{name} dw {values}')
for cls in sorted(set(c[1] for c in controls)):
    out.append(f"class{cls} du '{cls}',0")
Path('src/controls-data.inc').write_text('\n'.join(out)+'\n')
