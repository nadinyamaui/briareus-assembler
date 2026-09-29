"""Run the real PE64 UI against the compile-time mock transport."""
from native_ui import *
import json

u.IsWindowEnabled.argtypes=[w.HWND]
u.PostMessageW.argtypes=[w.HWND,w.UINT,w.WPARAM,w.LPARAM]

def launch():
    p=subprocess.Popen([str(Path('build/Briareus-test.exe').resolve())])
    wait_for(lambda: u.FindWindowW('BriareusAssemblerWindow',None))
    h=u.FindWindowW('BriareusAssemblerWindow',None)
    wait_for(lambda: control(h,102))
    return p,h

def close(p,h):
    u.PostMessageW(h,0x10,0,0)
    p.wait(timeout=5)

def pair_fixture(h,mode='a'):
    set_text(h,100,'https://example.com')
    set_text(h,101,'brm_'+mode*43)
    click(h,102)
    wait_for(lambda: u.SendMessageW(control(h,200),0x18b,0,0)==2)

Path('build/test-requests.jsonl').write_text('')
p,h=launch()
try:
    pair_fixture(h)
    select(h,200,0)
    wait_for(lambda:u.SendMessageW(control(h,205),0x18b,0,0)==2)
    set_text(h,204,'mobile')
    assert u.SendMessageW(control(h,205),0x18b,0,0)==1
    set_text(h,204,'')
    select(h,205,0)
    wait_for(lambda:'stable session ID' in text(control(h,206)))
    assert 'MUST NOT RENDER' not in text(control(h,206))
    assert 'España — 日本語 🚀' in text(control(h,206))
    screenshot(h,'build/conversation.png')
    message='Quote "hello" \\ path\nEspaña 日本語 🚀'
    set_text(h,207,message)
    click(h,208)
    wait_for(lambda:text(control(h,207))=='')
    wait_for(lambda:u.IsWindowEnabled(control(h,209)))
    requests=[json.loads(x) for x in Path('build/test-requests.jsonl').read_text(encoding='utf-8').splitlines()]
    assert sum(x.get('text')==message for x in requests)==1, requests
    assert any(x.get('sessionId')=='session-1' and x.get('text')==message for x in requests)
    set_text(h,207,'Create native tests')
    click(h,209)
    wait_for(lambda:text(control(h,207))=='')
    wait_for(lambda:u.IsWindowEnabled(control(h,209)))
    requests=[json.loads(x) for x in Path('build/test-requests.jsonl').read_text(encoding='utf-8').splitlines()]
    assert {'repo':'demo/briareus','prompt':'Create native tests'} in requests
    set_text(h,207,'Unsent draft')
    click(h,210);click(h,213)
    assert text(control(h,207))=='Unsent draft'
    select(h,203,1,True)
    wait_for(lambda:u.SendMessageW(control(h,205),0x18b,0,0)==1)
    select(h,205,0)
    wait_for(lambda:'Fix selection when refreshing' in text(control(h,206)))
    screenshot(h,'build/pull.png')
    click(h,210)
    select(h,211,5,True)
    assert json.loads(text(control(h,207)))=={'repo':'demo/briareus','pr':42}
    click(h,212)
    wait_for(lambda:'src/selection.asm' in text(control(h,206)))
    assert '+ mov eax,[selectedRow]' in text(control(h,206))
    screenshot(h,'build/files.png')
    set_text(h,207,'{"broken":}')
    click(h,212)
    assert 'valid JSON object' in text(control(h,23))
    select(h,203,2,True)
    wait_for(lambda:u.SendMessageW(control(h,205),0x18b,0,0)==1)
    select(h,205,0)
    assert 'Refresh should preserve focus.' in text(control(h,206))
finally:
    close(p,h)

p,h=launch()
try:
    pair_fixture(h,'r')
    assert not u.IsWindowEnabled(control(h,208))
    assert not u.IsWindowEnabled(control(h,209))
    assert u.SendMessageW(control(h,211),0x146,0,0)==10
    screenshot(h,'build/readonly.png')
finally:
    close(p,h)
print('PASS: pairing, projects, search, Unicode transcript, hidden setup, exact-once message, start, draft preservation, PR detail, diffs, invalid JSON, issues, read-only gates')
