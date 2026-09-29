"""Resize the native fixture app; assert bounds, navigation and draft continuity."""
from native_ui import *

u.MoveWindow.argtypes=[w.HWND,c.c_int,c.c_int,c.c_int,c.c_int,w.BOOL]
u.GetClientRect.argtypes=[w.HWND,c.POINTER(w.RECT)]
u.ScreenToClient.argtypes=[w.HWND,c.POINTER(w.POINT)]
u.IsWindowVisible.argtypes=[w.HWND]
u.IsWindowEnabled.argtypes=[w.HWND]
u.PostMessageW.argtypes=[w.HWND,w.UINT,w.WPARAM,w.LPARAM]
ids=[10,11,12,13,14,20,21,22,23,100,101,102,*range(200,218)]

def visible(h,i): return bool(u.IsWindowVisible(control(h,i)))
def bounds(h,i):
    r=w.RECT();u.GetWindowRect(control(h,i),c.byref(r))
    a=w.POINT(r.left,r.top);b=w.POINT(r.right,r.bottom)
    u.ScreenToClient(h,c.byref(a));u.ScreenToClient(h,c.byref(b))
    return a.x,a.y,b.x,b.y

def check(h):
    client=w.RECT();u.GetClientRect(h,c.byref(client))
    shown=[]
    for i in ids:
        if not visible(h,i): continue
        x,y,r,b=bounds(h,i)
        assert 0<=x<r<=client.right and 0<=y<b<=client.bottom,(i,(x,y,r,b),client.right,client.bottom)
        shown.append((i,(x,y,r,b)))
    for idx,(i,a) in enumerate(shown):
        for j,b in shown[idx+1:]:
            assert min(a[2],b[2])<=max(a[0],b[0]) or min(a[3],b[3])<=max(a[1],b[1]),('overlap',i,j,a,b)

def resize(h,width,height):
    u.MoveWindow(h,60,40,width,height,True)
    check(h)

p=subprocess.Popen([str(Path('build/Briareus-test.exe').resolve())])
wait_for(lambda:u.FindWindowW('BriareusAssemblerWindow',None))
h=u.FindWindowW('BriareusAssemblerWindow',None)
try:
    wait_for(lambda:control(h,102))
    for width,height in [(480,680),(820,700),(1200,820),(1600,1000)]:
        resize(h,width,height)
    resize(h,480,680);screenshot(h,'build/responsive-pairing.png')
    set_text(h,100,'https://example.com');set_text(h,101,'brm_'+'a'*43);click(h,102)
    wait_for(lambda:u.SendMessageW(control(h,200),0x18b,0,0)==2)
    check(h)
    assert visible(h,200) and not visible(h,205) and not visible(h,206)
    select(h,200,0)
    wait_for(lambda:u.SendMessageW(control(h,205),0x18b,0,0)==2)
    assert not visible(h,200) and visible(h,205)
    check(h);screenshot(h,'build/responsive-list.png')
    select(h,205,0)
    wait_for(lambda:'stable session ID' in text(control(h,206)))
    wait_for(lambda:u.IsWindowEnabled(control(h,208)))
    assert visible(h,206) and not visible(h,205)
    set_text(h,207,'Keep this draft while resizing — España')
    for width,height,name in [(480,680,'compact'),(680,740,'narrow'),(820,700,'medium'),(1000,760,'medium-wide'),(1115,760,'boundary-low'),(1116,760,'boundary-high'),(1200,820,'wide'),(1600,1000,'large')]:
        resize(h,width,height)
        assert text(control(h,207))=='Keep this draft while resizing — España'
        if name in ('compact','medium','wide'):
            screenshot(h,f'build/responsive-{name}.png')
    resize(h,480,680)
    click(h,215);check(h);assert visible(h,200)
    click(h,216);check(h);assert visible(h,205)
    click(h,217);check(h);assert visible(h,206)
    assert text(control(h,207))=='Keep this draft while resizing — España'
    click(h,210);check(h)
    set_text(h,207,'{"repo":"demo/briareus"}')
    for width in (480,820,1200):
        resize(h,width,760)
        assert text(control(h,207))=='{"repo":"demo/briareus"}'
    resize(h,480,680);screenshot(h,'build/responsive-api.png')
    click(h,213);check(h)
    assert text(control(h,207))=='Keep this draft while resizing — España'
finally:
    u.PostMessageW(h,0x10,0,0);p.wait(timeout=5)
print('PASS: responsive bounds and non-overlap, compact navigation, 8 window sizes, breakpoint transitions, draft and JSON preservation')
