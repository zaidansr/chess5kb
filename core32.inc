; Hand-written 32-bit x86 chess core, FASM syntax.
; EBP points to >=132 writable bytes, initially uninitialized.
; Board: 0x88, a8=0, h1=119. 0 empty, P/N/B/R/Q/K=1..6.
; Black bit8; moved bit16. State[128]=side0/8, [129]=EP square/88h.
; State[130]=promotion type2..5, defaults/resets to queen5.
; core_init: initialize. core_play EAX=from EDX=to, apply & swap side.
; core_legal EAX=from EDX=to -> EAX=0/1.
; core_moves EDI=word-pair buffer (512 bytes) -> EDI=end; other regs saved.
; core_check -> EAX=current side checked (1/0); other regs saved.
; core_best ECX=plies -> EAX=from|(to<<8), FFFFh if no legal moves.
; Bot promotion auto-queens. No fifty-move/repetition/insufficient-material draw.
; Public routines preserve EBP. core_play/core_init preserve all registers.

core_init:
 pushad
 mov edi,ebp
 xor eax,eax
 mov ecx,33
 rep stosd
 mov ecx,8
.back:
 mov al,[core_back+ecx-1]
 mov [ebp+ecx+111],al
 or al,8
 mov [ebp+ecx-1],al
 mov byte[ebp+ecx+15],9
 mov byte[ebp+ecx+95],1
 loop .back
 mov byte[ebp+129],88h
 mov byte[ebp+130],5
 popad
 ret

; Enumerate pseudo-legal moves, deliberately including attacks on enemy king.
core_pseudo:
 pushad
 sub esp,8
 xor esi,esi
.square:
 mov bl,[ebp+esi]
 movzx eax,bl
 and eax,7
 jz .next
 mov [esp],eax
 mov al,bl
 xor al,[ebp+128]
 test al,8
 jnz .next
 cmp dword[esp],1
 je .pawn
 mov ecx,8
 mov eax,16
 cmp dword[esp],2
 jne .notknight
 xor ecx,ecx
 mov eax,8
.notknight:
 cmp dword[esp],3
 jne .notbishop
 mov eax,12
.notbishop:
 cmp dword[esp],4
 jne .notrook
 mov ecx,12
.notrook:
 mov [esp+4],eax
.direction:
 movsx eax,byte[core_dirs+ecx]
 mov edx,esi
.ray:
 add edx,eax
 test edx,88h
 jnz .nextdir
 mov bh,[ebp+edx]
 test bh,7
 jz .emit
 xor bh,bl
 test bh,8
 jz .nextdir
 mov [edi],si
 mov [edi+1],dl
 add edi,2
 jmp .nextdir
.emit:
 mov [edi],si
 mov [edi+1],dl
 add edi,2
 cmp dword[esp],2
 je .nextdir
 cmp dword[esp],6
 jne .ray
.nextdir:
 inc ecx
 cmp ecx,[esp+4]
 jb .direction
 cmp dword[esp],6
 jne .next
 test bl,16
 jnz .next
 mov al,bl
 and al,15
 sub al,2
 cmp [ebp+esi+3],al
 jne .queenCastle
 cmp byte[ebp+esi+1],0
 jne .queenCastle
 cmp byte[ebp+esi+2],0
 jne .queenCastle
 lea edx,[esi+2]
 mov [edi],si
 mov [edi+1],dl
 add edi,2
.queenCastle:
 cmp [ebp+esi-4],al
 jne .next
 cmp byte[ebp+esi-1],0
 jne .next
 cmp byte[ebp+esi-2],0
 jne .next
 cmp byte[ebp+esi-3],0
 jne .next
 lea edx,[esi-2]
 jmp .pawnEmit
.pawn:
 mov eax,-16
 test bl,8
 jz .pawnDir
 neg eax
.pawnDir:
 lea edx,[esi+eax]
 test edx,88h
 jnz .next
 cmp byte[ebp+edx],0
 jne .captures
 mov [edi],si
 mov [edi+1],dl
 add edi,2
 test bl,16
 jnz .captures
 add edx,eax
 cmp byte[ebp+edx],0
 jne .captures
 mov [edi],si
 mov [edi+1],dl
 add edi,2
.captures:
 lea edx,[esi+eax-1]
 mov ecx,2
.capture:
 test edx,88h
 jnz .captureNext
 cmp dl,[ebp+129]
 je .captureEmit
 mov bh,[ebp+edx]
 test bh,7
 jz .captureNext
 xor bh,bl
 test bh,8
 jz .captureNext
.captureEmit:
 mov [edi],si
 mov [edi+1],dl
 add edi,2
.captureNext:
 add edx,2
 loop .capture
 jmp .next
.pawnEmit:
 mov [edi],si
 mov [edi+1],dl
 add edi,2
.next:
 inc esi
 test esi,8
 jz .square
 add esi,8
 cmp esi,128
 jb .square
 mov [esp+8],edi
 add esp,8
 popad
 ret

core_play:
 pushad
 mov bl,[ebp+eax]
 mov cl,[ebp+129]
 mov byte[ebp+129],88h
 mov byte[ebp+eax],0
 mov bh,bl
 and bh,7
 cmp bh,1
 jne .king
 cmp dl,cl
 jne .double
 mov ecx,16
 test bl,8
 jz .ep
 neg ecx
.ep:
 add ecx,edx
 mov byte[ebp+ecx],0
.double:
 mov ecx,edx
 sub ecx,eax
 cmp ecx,32
 je .setEp
 cmp ecx,-32
 jne .promote
.setEp:
 lea ecx,[eax+edx]
 shr ecx,1
 mov [ebp+129],cl
.promote:
 cmp edx,16
 jb .queen
 cmp edx,112
 jb .write
.queen:
 mov cl,[ebp+130]
 cmp cl,2
 jb .defaultQueen
 cmp cl,5
 jbe .promoteTo
.defaultQueen:
 mov cl,5
.promoteTo:
 and bl,8
 or bl,cl
 mov byte[ebp+130],5
 jmp .write
.king:
 cmp bh,6
 jne .write
 mov ecx,edx
 sub ecx,eax
 cmp ecx,2
 je .kingCastle
 cmp ecx,-2
 jne .write
 mov cl,[ebp+eax-4]
 or cl,16
 mov [ebp+eax-1],cl
 mov byte[ebp+eax-4],0
 jmp .write
.kingCastle:
 mov cl,[ebp+eax+3]
 or cl,16
 mov [ebp+eax+1],cl
 mov byte[ebp+eax+3],0
.write:
 or bl,16
 mov [ebp+edx],bl
 xor byte[ebp+128],8
 popad
 ret

core_check:
 pushad
 sub esp,512
 mov al,[ebp+128]
 or al,6
 xor esi,esi
.king:
 mov dl,[ebp+esi]
 and dl,15
 cmp dl,al
 je .found
 inc esi
 cmp esi,128
 jb .king
 mov eax,1
 jmp .return
.found:
 xor byte[ebp+128],8
 mov edi,esp
 call core_pseudo
 xor byte[ebp+128],8
 mov edx,esp
 mov eax,esi
.scan:
 cmp edx,edi
 jae .safe
 cmp [edx+1],al
 je .check
 add edx,2
 jmp .scan
.safe:
 xor eax,eax
 jmp .return
.check:
 mov eax,1
.return:
 mov [esp+540],eax
 add esp,512
 popad
 ret

; Frame: pseudo moves0..511, state512..643, end644,ptr648,out652,move656.
core_moves:
 pushad
 sub esp,660
 mov esi,ebp
 lea edi,[esp+512]
 mov ecx,33
 rep movsd
 mov edi,esp
 call core_pseudo
 mov [esp+644],edi
 mov eax,esp
 mov [esp+648],eax
 mov eax,[esp+660]
 mov [esp+652],eax
.loop:
 mov esi,[esp+648]
 cmp esi,[esp+644]
 jae .done
 movzx eax,word[esi]
 mov [esp+656],eax
 movzx edx,ah
 and eax,127
 mov bl,[ebp+edx]
 and bl,7
 cmp bl,6
 je .skip
 mov bl,[ebp+eax]
 and bl,7
 cmp bl,6
 jne .ordinary
 mov ecx,edx
 sub ecx,eax
 cmp ecx,2
 je .castle
 cmp ecx,-2
 jne .ordinary
.castle:
 call core_check
 test eax,eax
 jnz .skip
 mov eax,[esp+656]
 movzx edx,ah
 and eax,127
 add edx,eax
 shr edx,1
 call core_play
 xor byte[ebp+128],8
 call core_check
 lea esi,[esp+512]
 mov edi,ebp
 mov ecx,33
 rep movsd
 test eax,eax
 jnz .skip
 mov eax,[esp+656]
 movzx edx,ah
 and eax,127
.ordinary:
 call core_play
 xor byte[ebp+128],8
 call core_check
 lea esi,[esp+512]
 mov edi,ebp
 mov ecx,33
 rep movsd
 test eax,eax
 jnz .skip
 mov edi,[esp+652]
 mov eax,[esp+656]
 stosw
 mov [esp+652],edi
.skip:
 add dword[esp+648],2
 jmp .loop
.done:
 mov eax,[esp+652]
 mov [esp+660],eax
 add esp,660
 popad
 ret

core_legal:
 pushad
 mov ebx,eax
 shl edx,8
 or ebx,edx
 sub esp,512
 mov edi,esp
 call core_moves
 mov esi,esp
 xor eax,eax
.scan:
 cmp esi,edi
 jae .done
 cmp [esi],bx
 je .yes
 add esi,2
 jmp .scan
.yes:
 inc eax
.done:
 mov [esp+540],eax
 add esp,512
 popad
 ret

core_eval:
 xor eax,eax
 xor ebx,ebx
.square:
 movzx edx,byte[ebp+ebx]
 mov ecx,edx
 and edx,7
 movzx edx,byte[core_values+edx]
 xor cl,[ebp+128]
 test cl,8
 jnz .enemy
 add eax,edx
 jmp .next
.enemy:
 sub eax,edx
.next:
 inc ebx
 test ebx,8
 jz .square
 add ebx,8
 cmp ebx,128
 jb .square
 ret

core_best:
 call core_search
 mov eax,ebx
 ret

; Frame moves512,state132,best644,score648,depth652,end656,ptr660.
core_search:
 pushad
 sub esp,664
 mov [esp+652],ecx
 mov ebx,0FFFFh
 test ecx,ecx
 jnz .branch
 call core_eval
 mov ebx,0FFFFh
 jmp .return
.branch:
 mov esi,ebp
 lea edi,[esp+512]
 mov ecx,33
 rep movsd
 mov edi,esp
 call core_moves
 mov [esp+656],edi
 mov eax,esp
 mov [esp+660],eax
 cmp edi,eax
 jne .hasmoves
 call core_check
 imul eax,-10000
 mov ebx,0FFFFh
 jmp .return
.hasmoves:
 mov dword[esp+648],-32767
 mov dword[esp+644],0FFFFh
.loop:
 mov esi,[esp+660]
 movzx eax,word[esi]
 movzx edx,ah
 and eax,127
 call core_play
 mov ecx,[esp+652]
 dec ecx
 call core_search
 neg eax
 lea esi,[esp+512]
 mov edi,ebp
 mov ecx,33
 rep movsd
 cmp eax,[esp+648]
 jle .next
 mov [esp+648],eax
 mov esi,[esp+660]
 movzx ebx,word[esi]
 mov [esp+644],ebx
.next:
 add dword[esp+660],2
 mov esi,[esp+660]
 cmp esi,[esp+656]
 jb .loop
 mov eax,[esp+648]
 mov ebx,[esp+644]
.return:
 mov [esp+692],eax
 mov [esp+680],ebx
 add esp,664
 popad
 ret

core_back db 4,2,3,5,6,3,2,4
core_dirs db -33,-31,-18,-14,14,18,31,33,-17,-15,15,17,-16,-1,1,16
core_values db 0,1,3,3,5,9,0,0
