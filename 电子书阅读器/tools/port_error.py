# -*- coding: utf-8 -*-
"""看串口打不开到底是「被占用」还是「设备不在」。"""
import ctypes
import ctypes.wintypes as wt

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
k32.CreateFileW.restype = wt.HANDLE
k32.CreateFileW.argtypes = [wt.LPCWSTR, wt.DWORD, wt.DWORD, wt.LPVOID,
                            wt.DWORD, wt.DWORD, wt.HANDLE]
GENERIC_READ, GENERIC_WRITE, OPEN_EXISTING = 0x80000000, 0x40000000, 3
INVALID = ctypes.c_void_p(-1).value

for port in ("COM3", "COM4", "COM5", "COM6"):
    h = k32.CreateFileW("\\\\.\\" + port, GENERIC_READ | GENERIC_WRITE, 0,
                        None, OPEN_EXISTING, 0, None)
    if not h or h == INVALID:
        err = ctypes.get_last_error()
        msg = {2: "设备不存在", 5: "被别的程序占用", 32: "被别的程序占用(共享冲突)"}
        print("%s: 打不开  err=%d  %s" % (port, err, msg.get(err, "")))
    else:
        print("%s: 可以打开" % port)
        k32.CloseHandle(h)
