# Symbol Comparison: queue.h (ref) vs queuepp.hpp (new)

## try_prepare_push

### ref (queue.h)

```x86asm
0000000000000000 <ref_spsc_try_prepare_push>:
   0:	mov    edx,DWORD PTR [rdi]
   2:	mov    eax,DWORD PTR [rdi+0x8]
   5:	sub    eax,edx
   7:	mov    ecx,DWORD PTR [rdi+0x4]
   a:	add    eax,ecx
   c:	test   eax,eax
   e:	mov    eax,0xffffffff
  13:	cmovg  eax,edx
  16:	xor    edx,edx
  18:	xor    ecx,ecx
  1a:	xor    edi,edi
  1c:	ret
  1d:	nop    DWORD PTR [rax]
```

### new (queuepp.hpp)

```x86asm
0000000000000020 <new_spsc_try_prepare_push>:
  20:	mov    edx,DWORD PTR [rdi+0x4]
  23:	mov    eax,DWORD PTR [rdi]
  25:	add    edx,DWORD PTR [rdi+0x8]
  28:	cmp    eax,edx
  2a:	mov    edx,0xffffffff
  2f:	cmove  eax,edx
  32:	xor    edx,edx
  34:	xor    edi,edi
  36:	ret
  37:	nop    WORD PTR [rax+rax*1+0x0]
```

## prepare_push

### ref (queue.h)

```x86asm
0000000000000040 <ref_spsc_prepare_push>:
  40:	push   rbp
  41:	mov    rbp,rsp
  44:	push   r14
  46:	push   r13
  48:	push   r12
  4a:	push   rbx
  4b:	mov    r14d,DWORD PTR [rdi]
  4e:	mov    r13d,DWORD PTR [rdi+0x8]
  52:	sub    r13d,r14d
  55:	mov    esi,DWORD PTR [rdi+0x4]
  58:	lea    eax,[r13+rsi*1+0x0]
  5d:	test   eax,eax
  5f:	jg     8d <ref_spsc_prepare_push+0x4d>
  61:	lea    r12,[rdi+0x4]
  65:	lea    rbx,[rdi+0xc]
  69:	nop    DWORD PTR [rax+0x0]
  70:	lock add DWORD PTR [rbx],0x1
  74:	mov    rdi,r12
  77:	call   7c <ref_spsc_prepare_push+0x3c>
  7c:	mov    esi,DWORD PTR [r12]
  80:	lock sub DWORD PTR [rbx],0x1
  84:	lea    eax,[r13+rsi*1+0x0]
  89:	test   eax,eax
  8b:	jle    70 <ref_spsc_prepare_push+0x30>
  8d:	pop    rbx
  8e:	mov    eax,r14d
  91:	pop    r12
  93:	pop    r13
  95:	pop    r14
  97:	pop    rbp
  98:	xor    esi,esi
  9a:	xor    edi,edi
  9c:	ret
  9d:	nop    DWORD PTR [rax]
```

### new (queuepp.hpp)

```x86asm
0000000000000190 <new_spsc_prepare_push>:
 190:	mov    esi,DWORD PTR [rdi+0x4]
 193:	mov    edx,DWORD PTR [rdi]
 195:	mov    eax,DWORD PTR [rdi+0x8]
 198:	add    eax,esi
 19a:	cmp    eax,edx
 19c:	je     1a2 <new_spsc_prepare_push+0x12>
 1a2:	mov    eax,edx
 1a4:	xor    edx,edx
 1a6:	xor    esi,esi
 1a8:	xor    edi,edi
 1aa:	ret
 1ab:	nop    DWORD PTR [rax+rax*1+0x0]
```

## commit_push

### ref (queue.h)

```x86asm
00000000000000a0 <ref_spsc_commit_push>:
  a0:	lock add DWORD PTR [rsi],0x1
  a4:	mov    eax,DWORD PTR [rsi+0xc]
  a7:	test   eax,eax
  a9:	jne    b8 <ref_spsc_commit_push+0x18>
  ab:	xor    eax,eax
  ad:	xor    esi,esi
  af:	xor    edi,edi
  b1:	ret
  b2:	nop    WORD PTR [rax+rax*1+0x0]
  b8:	mov    rdi,rsi
  bb:	jmp    c0 <ref_spsc_try_prepare_consume>
```

### new (queuepp.hpp)

```x86asm
00000000000001b0 <new_spsc_commit_push>:
 1b0:	lock add DWORD PTR [rsi],0x1
 1b4:	mov    eax,DWORD PTR [rsi+0xc]
 1b7:	test   eax,eax
 1b9:	jne    1bf <new_spsc_commit_push+0xf>
 1bf:	xor    eax,eax
 1c1:	xor    esi,esi
 1c3:	xor    edi,edi
 1c5:	ret
 1c6:	cs nop WORD PTR [rax+rax*1+0x0]
```

## try_prepare_consume

### ref (queue.h)

```x86asm
00000000000000c0 <ref_spsc_try_prepare_consume>:
  c0:	mov    eax,DWORD PTR [rdi+0x4]
  c3:	mov    edx,DWORD PTR [rdi]
  c5:	cmp    eax,edx
  c7:	mov    edx,0xffffffff
  cc:	cmove  eax,edx
  cf:	xor    edx,edx
  d1:	xor    edi,edi
  d3:	ret
  d4:	nop
  d5:	data16 cs nop WORD PTR [rax+rax*1+0x0]
```

### new (queuepp.hpp)

```x86asm
00000000000000e0 <new_spsc_try_prepare_consume>:
  e0:	mov    edx,DWORD PTR [rdi]
  e2:	mov    eax,DWORD PTR [rdi+0x4]
  e5:	cmp    edx,eax
  e7:	mov    edx,0xffffffff
  ec:	cmove  eax,edx
  ef:	xor    edx,edx
  f1:	xor    edi,edi
  f3:	ret
  f4:	nop
  f5:	data16 cs nop WORD PTR [rax+rax*1+0x0]
```

## prepare_consume

### ref (queue.h)

```x86asm
0000000000000100 <ref_spsc_prepare_consume>:
 100:	push   rbp
 101:	mov    rbp,rsp
 104:	push   r14
 106:	push   r13
 108:	push   r12
 10a:	push   rbx
 10b:	mov    r14d,DWORD PTR [rdi+0x4]
 10f:	mov    r13d,DWORD PTR [rdi]
 112:	cmp    r14d,r13d
 115:	jne    149 <ref_spsc_prepare_consume+0x49>
 117:	mov    r12,rdi
 11a:	mov    esi,r14d
 11d:	lea    rbx,[rdi+0xc]
 121:	nop    DWORD PTR [rax+0x0]
 125:	data16 cs nop WORD PTR [rax+rax*1+0x0]
 130:	lock add DWORD PTR [rbx],0x1
 134:	mov    rdi,r12
 137:	call   13c <ref_spsc_prepare_consume+0x3c>
 13c:	mov    esi,DWORD PTR [r12]
 140:	lock sub DWORD PTR [rbx],0x1
 144:	cmp    r13d,esi
 147:	je     130 <ref_spsc_prepare_consume+0x30>
 149:	pop    rbx
 14a:	mov    eax,r14d
 14d:	pop    r12
 14f:	pop    r13
 151:	pop    r14
 153:	pop    rbp
 154:	xor    esi,esi
 156:	xor    edi,edi
 158:	ret
 159:	nop    DWORD PTR [rax+0x0]
```

### new (queuepp.hpp)

```x86asm
00000000000001d0 <new_spsc_prepare_consume>:
 1d0:	mov    esi,DWORD PTR [rdi]
 1d2:	mov    edx,DWORD PTR [rdi+0x4]
 1d5:	cmp    esi,edx
 1d7:	je     1dd <new_spsc_prepare_consume+0xd>
 1dd:	mov    eax,edx
 1df:	xor    edx,edx
 1e1:	xor    esi,esi
 1e3:	xor    edi,edi
 1e5:	ret
 1e6:	cs nop WORD PTR [rax+rax*1+0x0]
```

## commit_consume

### ref (queue.h)

```x86asm
0000000000000160 <ref_spsc_commit_consume>:
 160:	lea    rdi,[rsi+0x4]
 164:	lock add DWORD PTR [rsi+0x4],0x1
 169:	mov    eax,DWORD PTR [rsi+0xc]
 16c:	test   eax,eax
 16e:	jne    180 <ref_spsc_commit_consume+0x20>
 170:	xor    eax,eax
 172:	xor    esi,esi
 174:	xor    edi,edi
 176:	ret
 177:	nop    WORD PTR [rax+rax*1+0x0]
 180:	jmp    185 <ref_spsc_commit_consume+0x25>
 185:	data16 cs nop WORD PTR [rax+rax*1+0x0]
```

### new (queuepp.hpp)

```x86asm
00000000000001f0 <new_spsc_commit_consume>:
 1f0:	lock add DWORD PTR [rsi+0x4],0x1
 1f5:	mov    eax,DWORD PTR [rsi+0xc]
 1f8:	test   eax,eax
 1fa:	jne    200 <new_spsc_commit_consume+0x10>
 200:	xor    eax,eax
 202:	xor    esi,esi
 204:	xor    edi,edi
 206:	ret
```

