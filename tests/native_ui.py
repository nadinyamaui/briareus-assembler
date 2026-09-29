"""Native smoke/integration tests against our window only; no desktop input injection."""
import ctypes as c
from ctypes import wintypes as w
from pathlib import Path
import subprocess
import time
import sys

u = c.WinDLL('user32', use_last_error=True)
g = c.WinDLL('gdi32', use_last_error=True)
u.FindWindowW.argtypes = [w.LPCWSTR, w.LPCWSTR]
u.FindWindowW.restype = w.HWND
u.GetDlgItem.argtypes = [w.HWND,c.c_int]
u.GetDlgItem.restype = w.HWND
u.SendMessageW.argtypes = [w.HWND,w.UINT,w.WPARAM,w.LPARAM]
u.SendMessageW.restype = c.c_ssize_t
u.SetWindowTextW.argtypes = [w.HWND,w.LPCWSTR]
u.GetWindowTextW.argtypes = [w.HWND,w.LPWSTR,c.c_int]
u.GetWindowTextLengthW.argtypes = [w.HWND]
u.GetWindowRect.argtypes = [w.HWND,c.POINTER(w.RECT)]
u.GetDC.argtypes=[w.HWND];u.GetDC.restype=w.HDC
u.ReleaseDC.argtypes=[w.HWND,w.HDC]
u.PrintWindow.argtypes=[w.HWND,w.HDC,w.UINT]
u.RedrawWindow.argtypes=[w.HWND,w.LPVOID,w.HRGN,w.UINT]
g.CreateCompatibleDC.argtypes=[w.HDC];g.CreateCompatibleDC.restype=w.HDC
g.CreateCompatibleBitmap.argtypes=[w.HDC,c.c_int,c.c_int];g.CreateCompatibleBitmap.restype=w.HBITMAP
g.SelectObject.argtypes=[w.HDC,w.HGDIOBJ];g.SelectObject.restype=w.HGDIOBJ
g.GetDIBits.argtypes=[w.HDC,w.HBITMAP,w.UINT,w.UINT,w.LPVOID,w.LPVOID,w.UINT]
g.DeleteObject.argtypes=[w.HGDIOBJ];g.DeleteDC.argtypes=[w.HDC]

def text(hwnd):
    b=c.create_unicode_buffer(u.SendMessageW(hwnd,0xE,0,0)+1)
    u.SendMessageW(hwnd,0xD,len(b),c.addressof(b))
    return b.value

def control(hwnd,ident): return u.GetDlgItem(hwnd,ident)
def click(hwnd,ident): u.SendMessageW(hwnd,0x111,ident,control(hwnd,ident))
def set_text(hwnd,ident,value):
    b=c.create_unicode_buffer(value)
    u.SendMessageW(control(hwnd,ident),0xC,0,c.addressof(b))
def select(hwnd,ident,index,combo=False):
    h=control(hwnd,ident)
    u.SendMessageW(h,0x14e if combo else 0x186,index,0)
    u.SendMessageW(hwnd,0x111,ident | (1<<16),h)

def wait_for(check,timeout=10):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        if check(): return
        time.sleep(.05)
    raise AssertionError('Timed out waiting for UI state')

def screenshot(hwnd,path):
    from PIL import Image
    # Settle WM_PAINT for every child before asking PrintWindow to draw the tree.
    u.RedrawWindow(hwnd,None,None,0x185)
    c.WinDLL('dwmapi').DwmFlush()
    r=w.RECT();u.GetWindowRect(hwnd,c.byref(r));width=r.right-r.left;height=r.bottom-r.top
    dc=u.GetDC(hwnd);mem=g.CreateCompatibleDC(dc);bmp=g.CreateCompatibleBitmap(dc,width,height)
    old=g.SelectObject(mem,bmp)
    assert u.PrintWindow(hwnd,mem,2), 'PrintWindow failed'
    class Info(c.Structure):
        _fields_=[('size',w.DWORD),('width',w.LONG),('height',w.LONG),('planes',w.WORD),('bits',w.WORD),('compression',w.DWORD),('image_size',w.DWORD),('x',w.LONG),('y',w.LONG),('colors',w.DWORD),('important',w.DWORD)]
    info=Info(40,width,-height,1,32,0,0,0,0,0,0)
    data=c.create_string_buffer(width*height*4)
    g.SelectObject(mem,old)
    assert g.GetDIBits(mem,bmp,0,height,data,c.byref(info),0)
    Image.frombuffer('RGB',(width,height),data,'raw','BGRX',0,1).save(path)
    g.DeleteObject(bmp);g.DeleteDC(mem);u.ReleaseDC(hwnd,dc)

if __name__=='__main__':
    hwnd=u.FindWindowW('BriareusAssemblerWindow',None)
    assert hwnd, 'Start Briareus.exe first'
    screenshot(hwnd,'build/pairing.png')
    set_text(hwnd,100,'http://example.com')
    set_text(hwnd,101,'brm_'+'a'*43)
    click(hwnd,102)
    assert 'HTTPS' in text(control(hwnd,23)), text(control(hwnd,23))
    set_text(hwnd,100,'https://example.com')
    set_text(hwnd,101,'invalid')
    click(hwnd,102)
    assert '43' in text(control(hwnd,23)), text(control(hwnd,23))
    set_text(hwnd,100,'');set_text(hwnd,101,'')
    print('PASS: native window, HTTP rejection and malformed token rejection')
