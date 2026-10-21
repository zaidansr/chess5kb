; Native Win32 GDI interface. Board and bot are entirely in this executable.
macro gui_call fn,[arg] {
 reverse
 push arg
 common
 call [fn]
}
G_HWND=132
G_DC=136
G_FONT=140
G_TEXTFONT=144
G_SELECTED=148
G_LAST=152
G_MODE=156
G_CHECK=160
G_END=164
G_MSG=172
G_PS=204
G_WC=268
G_GLYPH=308
G_MOVES=312

gui_entry:
 mov ebp,state
 gui_call GetModuleHandleA,0
 mov [ebp+G_WC+16],eax
 mov dword[ebp+G_WC+4],gui_proc
 mov dword[ebp+G_WC+36],gui_class
 gui_call LoadCursorA,0,32512
 mov [ebp+G_WC+24],eax
 lea eax,[ebp+G_WC]
 gui_call RegisterClassA,eax
 gui_call CreateWindowExA,0,gui_class,gui_title,00CA0000h,80000000h,80000000h,756,609,0,0,dword[ebp+G_WC+16],0
 mov [ebp+G_HWND],eax
 gui_call CreateFontA,54,0,0,0,400,0,0,0,1,0,0,4,0,gui_face
 mov [ebp+G_FONT],eax
 gui_call CreateFontA,20,0,0,0,400,0,0,0,1,0,0,4,0,gui_ui_face
 mov [ebp+G_TEXTFONT],eax
 call gui_reset
 gui_call ShowWindow,dword[ebp+G_HWND],5
.loop:
 lea esi,[ebp+G_MSG]
 gui_call GetMessageA,esi,0,0,0
 test eax,eax
 jle .exit
 gui_call TranslateMessage,esi
 gui_call DispatchMessageA,esi
 jmp .loop
.exit:
 gui_call ExitProcess,0

gui_proc:
 push ebp ebx esi edi
 mov ebp,state
 mov eax,[esp+24]
 cmp eax,15
 je .paint
 cmp eax,201h
 je .click
 cmp eax,400h
 je .bot
 cmp eax,2
 je .close
 push dword[esp+32]
 push dword[esp+32]
 push dword[esp+32]
 push dword[esp+32]
 call [DefWindowProcA]
 jmp .return
.close:
 gui_call PostQuitMessage,0
 jmp .zero
.bot:
 cmp byte[ebp+128],8
 jne .zero
 gui_call UpdateWindow,dword[ebp+G_HWND]
 mov ecx,[ebp+G_MODE]
 test ecx,ecx
 jz .zero
 mov byte[ebp+130],5
 call core_best
 cmp ax,0FFFFh
 je .zero
 mov edx,eax
 shr edx,8
 and eax,127
 jmp .play
.click:
 mov eax,[esp+32]
 movzx esi,ax
 shr eax,16
 mov edi,eax
 cmp esi,520
 jae .sidebar
 sub esi,20
 cmp esi,480
 jae .zero
 sub edi,60
 cmp edi,480
 jae .zero
 mov eax,edi
 xor edx,edx
 mov ecx,60
 div ecx
 shl eax,4
 mov ebx,eax
 mov eax,esi
 xor edx,edx
 div ecx
 add ebx,eax
 cmp byte[ebp+128],8
 jne .human
 cmp dword[ebp+G_MODE],0
 jne .zero
.human:
 lea eax,[ebp+G_MOVES]
 cmp eax,[ebp+G_END]
 je .zero
 mov eax,[ebp+G_SELECTED]
 cmp eax,-1
 je .select
 mov edx,ebx
 push eax edx
 call core_legal
 test eax,eax
 pop edx eax
 jnz .play
.select:
 mov al,[ebp+ebx]
 test al,7
 jz .deselect
 xor al,[ebp+128]
 test al,8
 jnz .deselect
 mov [ebp+G_SELECTED],ebx
 jmp .redraw
.deselect:
 or dword[ebp+G_SELECTED],-1
 jmp .redraw
.sidebar:
 sub edi,70
 cmp edi,216
 jae .promotion
 mov eax,edi
 xor edx,edx
 mov ecx,45
 div ecx
 cmp edx,36
 jae .zero
 cmp eax,4
 je .restart
 mov [ebp+G_MODE],eax
.restart:
 call gui_reset
 jmp .redraw
.promotion:
 add edi,70-470
 cmp edi,36
 jae .zero
 sub esi,540
 cmp esi,160
 jae .zero
 mov eax,esi
 xor edx,edx
 mov ecx,40
 div ecx
 add al,2
 mov [ebp+130],al
 jmp .redraw
.play:
 mov [ebp+G_LAST],al
 mov [ebp+G_LAST+1],dl
 call core_play
 or dword[ebp+G_SELECTED],-1
 call gui_refresh
.redraw:
 gui_call InvalidateRect,dword[ebp+G_HWND],0,0
 cmp byte[ebp+128],8
 jne .zero
 cmp dword[ebp+G_MODE],0
 je .zero
 gui_call PostMessageA,dword[ebp+G_HWND],400h,0,0
.zero:
 xor eax,eax
.return:
 pop edi esi ebx ebp
 ret 16
.paint:
 lea eax,[ebp+G_PS]
 gui_call BeginPaint,dword[ebp+G_HWND],eax
 mov [ebp+G_DC],eax
 gui_call GetStockObject,18
 gui_call SelectObject,dword[ebp+G_DC],eax
 gui_call GetStockObject,8
 gui_call SelectObject,dword[ebp+G_DC],eax
 gui_call SetBkMode,dword[ebp+G_DC],1
 xor esi,esi
 xor edi,edi
 mov ecx,740
 mov edx,570
 mov eax,002B2E30h
 call gui_box
 gui_call SelectObject,dword[ebp+G_DC],dword[ebp+G_FONT]
 xor ebx,ebx
 mov edi,60
.row:
 mov esi,20
.square:
 mov eax,ebx
 shr eax,4
 xor eax,ebx
 and eax,1
 neg eax
 and eax,00847898h
 xor eax,00D2EEEEh
 cmp bl,[ebp+G_LAST]
 je .highlight
 cmp bl,[ebp+G_LAST+1]
 jne .selected
.highlight:
 mov eax,0062C8C8h
.selected:
 cmp ebx,[ebp+G_SELECTED]
 jne .checked
 mov eax,0049C5D8h
.checked:
 cmp dword[ebp+G_CHECK],0
 je .squarebg
 mov dl,[ebp+ebx]
 and dl,15
 mov cl,[ebp+128]
 or cl,6
 cmp dl,cl
 jne .squarebg
 mov eax,005C5CDEh
.squarebg:
 mov ecx,60
 mov edx,60
 call gui_box
 movzx eax,byte[ebp+ebx]
 mov edx,eax
 and eax,7
 jz .mark
 neg eax
 add eax,9824
 mov [ebp+G_GLYPH],ax
 mov eax,00272B2Dh
 call gui_piece
 test byte[ebp+ebx],8
 jnz .mark
 sub esi,2
 sub edi,2
 mov eax,00FFFFFFh
 call gui_piece
 add esi,2
 add edi,2
.mark:
 lea edx,[ebp+G_MOVES]
.scan:
 cmp edx,[ebp+G_END]
 jae .next
 mov eax,[ebp+G_SELECTED]
 cmp al,[edx]
 jne .scanNext
 cmp bl,[edx+1]
 jne .scanNext
 push esi edi
 cmp byte[ebp+ebx],0
 je .dot
 add esi,20
 sub edi,20
.dot:
 mov eax,00456739h
 gui_call SetDCBrushColor,dword[ebp+G_DC],eax
 lea eax,[esi+24]
 lea ecx,[esi+36]
 lea edx,[edi+36]
 add edi,24
 gui_call Ellipse,dword[ebp+G_DC],eax,edi,ecx,edx
 pop edi esi
 jmp .next
.scanNext:
 add edx,2
 jmp .scan
.next:
 add esi,60
 inc ebx
 test bl,8
 jz .square
 add ebx,8
 add edi,60
 cmp ebx,128
 jb .row
 gui_call SelectObject,dword[ebp+G_DC],dword[ebp+G_TEXTFONT]
 gui_call SetTextColor,dword[ebp+G_DC],00A8A8A8h
 xor ebx,ebx
.coordinates:
 mov esi,4
 mov edi,ebx
 imul edi,60
 add edi,79
 mov eax,'8'
 sub eax,ebx
 mov [ebp+G_GLYPH],ax
 lea eax,[ebp+G_GLYPH]
 call gui_text
 mov esi,ebx
 imul esi,60
 add esi,46
 mov edi,542
 lea eax,[ebx+'a']
 mov [ebp+G_GLYPH],ax
 lea eax,[ebp+G_GLYPH]
 call gui_text
 inc ebx
 cmp ebx,8
 jb .coordinates
 gui_call SetTextColor,dword[ebp+G_DC],00ECEDEEh
 mov esi,20
 mov edi,19
 mov eax,gui_title
 call gui_text
 mov esi,540
 mov eax,gui_new
 call gui_text
 xor ebx,ebx
 mov edi,70
.buttons:
 mov esi,532
 mov eax,003C403Fh
 cmp ebx,[ebp+G_MODE]
 jne .buttonbg
 mov eax,004F7855h
.buttonbg:
 mov ecx,188
 mov edx,36
 call gui_box
 mov eax,ebx
 shl eax,4
 add eax,gui_modes
 add esi,12
 add edi,6
 call gui_text
 sub edi,6
 add edi,45
 inc ebx
 cmp ebx,5
 jb .buttons
 mov esi,540
 mov edi,317
 mov eax,gui_white
 cmp byte[ebp+128],0
 je .turn
 mov eax,gui_black
.turn:
 call gui_text
 mov edi,349
 mov eax,gui_check
 cmp dword[ebp+G_CHECK],0
 jne .status
 mov eax,gui_select
.status:
 lea ecx,[ebp+G_MOVES]
 cmp ecx,[ebp+G_END]
 jne .live
 mov eax,gui_mate
 cmp dword[ebp+G_CHECK],0
 jne .statusdraw
 mov eax,gui_stale
 jmp .statusdraw
.live:
 cmp byte[ebp+128],8
 jne .statusdraw
 cmp dword[ebp+G_MODE],0
 je .statusdraw
 mov eax,gui_think
.statusdraw:
 call gui_text
 mov edi,430
 mov eax,gui_promote
 call gui_text
 xor ebx,ebx
 mov esi,540
 mov edi,470
.promotions:
 lea eax,[ebx+2]
 cmp al,[ebp+130]
 mov eax,003C403Fh
 jne .promobg
 mov eax,004F7855h
.promobg:
 mov ecx,36
 mov edx,36
 call gui_box
 lea eax,[gui_promos+ebx*2]
 add esi,10
 add edi,6
 call gui_text
 sub edi,6
 add esi,30
 inc ebx
 cmp ebx,4
 jb .promotions
 lea eax,[ebp+G_PS]
 gui_call EndPaint,dword[ebp+G_HWND],eax
 jmp .zero

gui_reset:
 call core_init
 or dword[ebp+G_SELECTED],-1
 or dword[ebp+G_LAST],-1
gui_refresh:
 call core_check
 mov [ebp+G_CHECK],eax
 lea edi,[ebp+G_MOVES]
 call core_moves
 mov [ebp+G_END],edi
 ret

gui_box:
 pushad
 push edx ecx
 gui_call SetDCBrushColor,dword[ebp+G_DC],eax
 pop ecx edx
 add ecx,esi
 add edx,edi
 gui_call Rectangle,dword[ebp+G_DC],esi,edi,ecx,edx
 popad
 ret

gui_piece:
 pushad
 gui_call SetTextColor,dword[ebp+G_DC],eax
 lea eax,[esi+5]
 lea edx,[edi-3]
 lea ecx,[ebp+G_GLYPH]
 gui_call TextOutW,dword[ebp+G_DC],eax,edx,ecx,1
 popad
 ret
gui_text:
 pushad
 mov edx,eax
 xor ecx,ecx
.length:
 cmp byte[eax+ecx],0
 je .out
 inc ecx
 jmp .length
.out:
 gui_call TextOutA,dword[ebp+G_DC],esi,edi,edx,ecx
 popad
 ret

gui_class db 'chess5kb',0
gui_title db 'chess5kb',0
gui_face db 'Segoe UI Symbol',0
gui_ui_face db 'Segoe UI',0
gui_new db 'NEW GAME',0
gui_modes:
 db 'Play a friend',0,0,0
 db 'Easy bot',0,0,0,0,0,0,0,0
 db 'Medium bot',0,0,0,0,0,0
 db 'Hard bot',0,0,0,0,0,0,0,0
 db 'Restart',0
gui_white db 'White to move',0
gui_black db 'Black to move',0
gui_check db 'Check!',0
gui_select db 'Select a piece',0
gui_mate db 'Checkmate',0
gui_stale db 'Stalemate',0
gui_think db 'Thinking...',0
gui_promote db 'Promote to',0
gui_promos db 'N',0,'B',0,'R',0,'Q',0





