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
  20:	mov    edx,DWORD PTR [rdi+0x8]
  23:	mov    eax,DWORD PTR [rdi+0x4]
  26:	add    edx,DWORD PTR [rdi]
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
 190:	push   rbp
 191:	mov    rbp,rsp
 194:	push   rbx
 195:	sub    rsp,0x28
 199:	mov    rax,QWORD PTR fs:0x28
 1a2:	mov    QWORD PTR [rbp-0x18],rax
 1a6:	mov    eax,DWORD PTR [rdi+0x8]
 1a9:	mov    DWORD PTR [rbp-0x1c],eax
 1ac:	mov    edx,DWORD PTR [rdi+0x4]
 1af:	add    eax,DWORD PTR [rdi]
 1b1:	cmp    edx,eax
 1b3:	je     1b9 <new_spsc_prepare_push+0x29>
 1b9:	mov    rax,QWORD PTR [rbp-0x18]
 1bd:	sub    rax,QWORD PTR fs:0x28
 1c6:	jne    1d6 <new_spsc_prepare_push+0x46>
 1c8:	mov    rbx,QWORD PTR [rbp-0x8]
 1cc:	mov    eax,edx
 1ce:	leave
 1cf:	xor    edx,edx
 1d1:	xor    esi,esi
 1d3:	xor    edi,edi
 1d5:	ret
 1d6:	call   1db <new_spsc_prepare_push+0x4b>
 1db:	nop    DWORD PTR [rax+rax*1+0x0]
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
00000000000001e0 <new_spsc_commit_push>:
 1e0:	lock add DWORD PTR [rsi+0x4],0x1
 1e5:	mov    eax,DWORD PTR [rsi+0xc]
 1e8:	test   eax,eax
 1ea:	jne    1f0 <new_spsc_commit_push+0x10>
 1f0:	xor    eax,eax
 1f2:	xor    esi,esi
 1f4:	xor    edi,edi
 1f6:	ret
 1f7:	nop    WORD PTR [rax+rax*1+0x0]
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
  e0:	mov    edx,DWORD PTR [rdi+0x4]
  e3:	mov    eax,DWORD PTR [rdi+0x8]
  e6:	cmp    edx,eax
  e8:	mov    edx,0xffffffff
  ed:	cmove  eax,edx
  f0:	xor    edx,edx
  f2:	xor    edi,edi
  f4:	ret
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
0000000000000200 <new_spsc_prepare_consume>:
 200:	push   rbp
 201:	mov    rbp,rsp
 204:	push   rbx
 205:	sub    rsp,0x28
 209:	mov    rax,QWORD PTR fs:0x28
 212:	mov    QWORD PTR [rbp-0x18],rax
 216:	mov    eax,DWORD PTR [rdi+0x4]
 219:	mov    DWORD PTR [rbp-0x1c],eax
 21c:	mov    eax,DWORD PTR [rdi+0x8]
 21f:	cmp    DWORD PTR [rbp-0x1c],eax
 222:	je     228 <new_spsc_prepare_consume+0x28>
 228:	mov    rdx,QWORD PTR [rbp-0x18]
 22c:	sub    rdx,QWORD PTR fs:0x28
 235:	jne    245 <new_spsc_prepare_consume+0x45>
 237:	mov    rbx,QWORD PTR [rbp-0x8]
 23b:	leave
 23c:	xor    edx,edx
 23e:	xor    ecx,ecx
 240:	xor    esi,esi
 242:	xor    edi,edi
 244:	ret
 245:	call   24a <new_spsc_prepare_consume+0x4a>
 24a:	nop    WORD PTR [rax+rax*1+0x0]
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
0000000000000250 <new_spsc_commit_consume>:
 250:	lock add DWORD PTR [rsi+0x8],0x1
 255:	mov    eax,DWORD PTR [rsi+0xc]
 258:	test   eax,eax
 25a:	jne    260 <new_spsc_commit_consume+0x10>
 260:	xor    eax,eax
 262:	xor    esi,esi
 264:	xor    edi,edi
 266:	ret
```

