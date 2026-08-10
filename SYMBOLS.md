# Symbol Comparison: queue.h (ref) vs queuepp.hpp (new)

## try_prepare_push

### ref (queue.h)

```asm
0000000000000000 <ref_spsc_try_prepare_push>:
   0:	mov    (%rdi),%edx
   2:	mov    0x8(%rdi),%eax
   5:	sub    %edx,%eax
   7:	mov    0x4(%rdi),%ecx
   a:	add    %ecx,%eax
   c:	test   %eax,%eax
   e:	mov    $0xffffffff,%eax
  13:	cmovg  %edx,%eax
  16:	xor    %edx,%edx
  18:	xor    %ecx,%ecx
  1a:	xor    %edi,%edi
  1c:	ret
  1d:	nopl   (%rax)
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_try_prepare_push>:
   0:	mov    0x8(%rdi),%edx
   3:	mov    0x4(%rdi),%ecx
   6:	add    %ecx,%edx
   8:	mov    (%rdi),%eax
   a:	cmp    %edx,%eax
   c:	mov    $0xffffffff,%edx
  11:	cmove  %edx,%eax
  14:	xor    %edx,%edx
  16:	xor    %ecx,%ecx
  18:	xor    %edi,%edi
  1a:	ret
```

## prepare_push

### ref (queue.h)

```asm
0000000000000020 <ref_spsc_prepare_push>:
  20:	push   %rbp
  21:	mov    %rsp,%rbp
  24:	push   %r14
  26:	push   %r13
  28:	push   %r12
  2a:	push   %rbx
  2b:	mov    (%rdi),%r14d
  2e:	mov    0x8(%rdi),%r13d
  32:	sub    %r14d,%r13d
  35:	mov    0x4(%rdi),%esi
  38:	lea    0x0(%r13,%rsi,1),%eax
  3d:	test   %eax,%eax
  3f:	jg     6d <ref_spsc_prepare_push+0x4d>
  41:	lea    0x4(%rdi),%r12
  45:	lea    0xc(%rdi),%rbx
  49:	nopl   0x0(%rax)
  50:	lock addl $0x1,(%rbx)
  54:	mov    %r12,%rdi
  57:	call   5c <ref_spsc_prepare_push+0x3c>
  5c:	mov    (%r12),%esi
  60:	lock subl $0x1,(%rbx)
  64:	lea    0x0(%r13,%rsi,1),%eax
  69:	test   %eax,%eax
  6b:	jle    50 <ref_spsc_prepare_push+0x30>
  6d:	pop    %rbx
  6e:	mov    %r14d,%eax
  71:	pop    %r12
  73:	pop    %r13
  75:	pop    %r14
  77:	pop    %rbp
  78:	xor    %esi,%esi
  7a:	xor    %edi,%edi
  7c:	ret
  7d:	nopl   (%rax)
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_prepare_push>:
   0:	mov    0x8(%rdi),%eax
   3:	mov    0x4(%rdi),%esi
   6:	lea    (%rax,%rsi,1),%ecx
   9:	mov    (%rdi),%edx
   b:	cmp    %edx,%ecx
   d:	je     1a <new_spsc_prepare_push+0x1a>
   f:	mov    %edx,%eax
  11:	xor    %edx,%edx
  13:	xor    %ecx,%ecx
  15:	xor    %esi,%esi
  17:	xor    %edi,%edi
  19:	ret
  1a:	push   %rbp
  1b:	sub    %edx,%eax
  1d:	mov    %rsp,%rbp
  20:	push   %rbx
  21:	mov    %eax,%ebx
  23:	sub    $0x18,%rsp
  27:	mov    %edx,-0x1c(%rbp)
  2a:	mov    %rdi,-0x18(%rbp)
  2e:	call   33 <new_spsc_prepare_push+0x33>
  33:	mov    -0x18(%rbp),%rdi
  37:	mov    -0x1c(%rbp),%edx
  3a:	mov    %eax,%esi
  3c:	mov    %ebx,%eax
  3e:	add    %esi,%eax
  40:	je     27 <new_spsc_prepare_push+0x27>
  42:	mov    -0x8(%rbp),%rbx
  46:	mov    %edx,%eax
  48:	leave
  49:	xor    %edx,%edx
  4b:	xor    %ecx,%ecx
  4d:	xor    %esi,%esi
  4f:	xor    %edi,%edi
  51:	ret
```

## commit_push

### ref (queue.h)

```asm
0000000000000080 <ref_spsc_commit_push>:
  80:	lock addl $0x1,(%rsi)
  84:	mov    0xc(%rsi),%eax
  87:	test   %eax,%eax
  89:	jne    98 <ref_spsc_commit_push+0x18>
  8b:	xor    %eax,%eax
  8d:	xor    %esi,%esi
  8f:	xor    %edi,%edi
  91:	ret
  92:	nopw   0x0(%rax,%rax,1)
  98:	mov    %rsi,%rdi
  9b:	jmp    a0 <ref_spsc_try_prepare_consume>
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_commit_push>:
   0:	lock addl $0x1,(%rsi)
   4:	mov    0xc(%rsi),%eax
   7:	test   %eax,%eax
   9:	jne    12 <new_spsc_commit_push+0x12>
   b:	xor    %eax,%eax
   d:	xor    %esi,%esi
   f:	xor    %edi,%edi
  11:	ret
  12:	mov    %rsi,%rdi
  15:	jmp    1a <new_spsc_commit_push+0x1a>
```

## try_prepare_consume

### ref (queue.h)

```asm
00000000000000a0 <ref_spsc_try_prepare_consume>:
  a0:	mov    0x4(%rdi),%eax
  a3:	mov    (%rdi),%edx
  a5:	cmp    %edx,%eax
  a7:	mov    $0xffffffff,%edx
  ac:	cmove  %edx,%eax
  af:	xor    %edx,%edx
  b1:	xor    %edi,%edi
  b3:	ret
  b4:	data16 cs nopw 0x0(%rax,%rax,1)
  bf:	nop
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_try_prepare_consume>:
   0:	mov    (%rdi),%edx
   2:	mov    0x4(%rdi),%eax
   5:	cmp    %eax,%edx
   7:	mov    $0xffffffff,%edx
   c:	cmove  %edx,%eax
   f:	xor    %edx,%edx
  11:	xor    %edi,%edi
  13:	ret
```

## prepare_consume

### ref (queue.h)

```asm
00000000000000c0 <ref_spsc_prepare_consume>:
  c0:	push   %rbp
  c1:	mov    %rsp,%rbp
  c4:	push   %r14
  c6:	push   %r13
  c8:	push   %r12
  ca:	push   %rbx
  cb:	mov    0x4(%rdi),%r14d
  cf:	mov    (%rdi),%r13d
  d2:	cmp    %r13d,%r14d
  d5:	jne    109 <ref_spsc_prepare_consume+0x49>
  d7:	mov    %rdi,%r12
  da:	lea    0xc(%rdi),%rbx
  de:	mov    %r14d,%esi
  e1:	data16 cs nopw 0x0(%rax,%rax,1)
  ec:	nopl   0x0(%rax)
  f0:	lock addl $0x1,(%rbx)
  f4:	mov    %r12,%rdi
  f7:	call   fc <ref_spsc_prepare_consume+0x3c>
  fc:	mov    (%r12),%esi
 100:	lock subl $0x1,(%rbx)
 104:	cmp    %esi,%r13d
 107:	je     f0 <ref_spsc_prepare_consume+0x30>
 109:	pop    %rbx
 10a:	mov    %r14d,%eax
 10d:	pop    %r12
 10f:	pop    %r13
 111:	pop    %r14
 113:	pop    %rbp
 114:	xor    %esi,%esi
 116:	xor    %edi,%edi
 118:	ret
 119:	nopl   0x0(%rax)
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_prepare_consume>:
   0:	mov    (%rdi),%esi
   2:	mov    0x4(%rdi),%edx
   5:	cmp    %edx,%esi
   7:	je     12 <new_spsc_prepare_consume+0x12>
   9:	mov    %edx,%eax
   b:	xor    %edx,%edx
   d:	xor    %esi,%esi
   f:	xor    %edi,%edi
  11:	ret
  12:	push   %rbp
  13:	mov    %rsp,%rbp
  16:	sub    $0x10,%rsp
  1a:	mov    %edx,-0xc(%rbp)
  1d:	mov    %rdi,-0x8(%rbp)
  21:	call   26 <new_spsc_prepare_consume+0x26>
  26:	mov    -0xc(%rbp),%edx
  29:	mov    -0x8(%rbp),%rdi
  2d:	mov    %eax,%esi
  2f:	cmp    %eax,%edx
  31:	je     1a <new_spsc_prepare_consume+0x1a>
  33:	leave
  34:	mov    %edx,%eax
  36:	xor    %edx,%edx
  38:	xor    %esi,%esi
  3a:	xor    %edi,%edi
  3c:	ret
```

## commit_consume

### ref (queue.h)

```asm
0000000000000120 <ref_spsc_commit_consume>:
 120:	lea    0x4(%rsi),%rdi
 124:	lock addl $0x1,0x4(%rsi)
 129:	mov    0xc(%rsi),%eax
 12c:	test   %eax,%eax
 12e:	jne    140 <ref_spsc_commit_consume+0x20>
 130:	xor    %eax,%eax
 132:	xor    %esi,%esi
 134:	xor    %edi,%edi
 136:	ret
 137:	nopw   0x0(%rax,%rax,1)
 140:	jmp    145 <ref_spsc_commit_consume+0x25>
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_commit_consume>:
   0:	lock addl $0x1,0x4(%rsi)
   5:	mov    0xc(%rsi),%eax
   8:	test   %eax,%eax
   a:	jne    13 <new_spsc_commit_consume+0x13>
   c:	xor    %eax,%eax
   e:	xor    %esi,%esi
  10:	xor    %edi,%edi
  12:	ret
  13:	mov    %rsi,%rdi
  16:	jmp    1b <new_spsc_commit_consume+0x1b>
```

