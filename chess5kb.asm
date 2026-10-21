; chess5kb: native Windows GUI chess with a strict 5,000-byte budget.
; Build with FASM 1.73.x. No runtime, assets, sidecars, or network access.
; Build.ps1 removes only zero padding after the last initialized byte.
format PE GUI 4.0 at 400000h
entry gui_entry
section '.chess' code readable writable executable
data import
ImportDescriptor:
dd 0,0,0,rva KernelName,rva KernelImports
dd 0,0,0,rva UserName,rva UserImports
dd 0,0,0,rva GdiName,rva GdiImports
dd 0,0,0,0,0
KernelName db 'kernel32.dll',0
UserName db 'user32.dll',0
GdiName db 'gdi32.dll',0
align 4
Imports:
KernelImports:
GetModuleHandleA dd rva hn_GetModuleHandleA
ExitProcess dd rva hn_ExitProcess
dd 0
UserImports:
RegisterClassA dd rva hn_RegisterClassA
CreateWindowExA dd rva hn_CreateWindowExA
ShowWindow dd rva hn_ShowWindow
UpdateWindow dd rva hn_UpdateWindow
LoadCursorA dd rva hn_LoadCursorA
GetMessageA dd rva hn_GetMessageA
TranslateMessage dd rva hn_TranslateMessage
DispatchMessageA dd rva hn_DispatchMessageA
DefWindowProcA dd rva hn_DefWindowProcA
BeginPaint dd rva hn_BeginPaint
EndPaint dd rva hn_EndPaint
InvalidateRect dd rva hn_InvalidateRect
PostMessageA dd rva hn_PostMessageA
PostQuitMessage dd rva hn_PostQuitMessage
dd 0
GdiImports:
CreateFontA dd rva hn_CreateFontA
SelectObject dd rva hn_SelectObject
GetStockObject dd rva hn_GetStockObject
SetDCBrushColor dd rva hn_SetDCBrushColor
SetTextColor dd rva hn_SetTextColor
SetBkMode dd rva hn_SetBkMode
Rectangle dd rva hn_Rectangle
Ellipse dd rva hn_Ellipse
TextOutW dd rva hn_TextOutW
TextOutA dd rva hn_TextOutA
dd 0
ImportsEnd:
macro hn name {
  align 2
  hn_#name dw 0
  db `name,0
}
hn GetModuleHandleA
hn ExitProcess
hn RegisterClassA
hn CreateWindowExA
hn ShowWindow
hn UpdateWindow
hn LoadCursorA
hn GetMessageA
hn TranslateMessage
hn DispatchMessageA
hn DefWindowProcA
hn BeginPaint
hn EndPaint
hn InvalidateRect
hn PostMessageA
hn PostQuitMessage
hn CreateFontA
hn SelectObject
hn GetStockObject
hn SetDCBrushColor
hn SetTextColor
hn SetBkMode
hn Rectangle
hn Ellipse
hn TextOutW
hn TextOutA
end data
HeaderAndImportsEnd:
include 'gui32.inc'
GuiEnd:
include 'core32.inc'
CoreEnd:
data fixups
end data
FileEnd:
state rb 8192
