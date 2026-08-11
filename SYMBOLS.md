# Symbol Comparison: queue.h (ref) vs queuepp.hpp (new)

## try_prepare_push

### ref (queue.h)

```asm
0000000000000000 <ref_spsc_try_prepare_push>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	add	x8, x0, #0x4
   c:	ldr	w9, [x0, #8]
  10:	ldr	w10, [x0]
  14:	ldar	w8, [x8]
  18:	sub	w9, w9, w10
  1c:	add	w8, w9, w8
  20:	cmp	w8, #0x1
  24:	csinv	w0, w10, wzr, ge	// ge = tcont
  28:	ldp	x29, x30, [sp], #16
  2c:	mov	x8, #0x0                   	// #0
  30:	mov	x9, #0x0                   	// #0
  34:	mov	x10, #0x0                   	// #0
  38:	ret
  3c:	nop
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_try_prepare_push>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	add	x8, x0, #0x4
   c:	ldr	w9, [x0, #8]
  10:	ldar	w8, [x8]
  14:	ldr	w10, [x0]
  18:	sub	w8, w8, w10
  1c:	cmn	w8, w9
  20:	csinv	w0, w10, wzr, ne	// ne = any
  24:	ldp	x29, x30, [sp], #16
  28:	mov	x8, #0x0                   	// #0
  2c:	mov	x9, #0x0                   	// #0
  30:	mov	x10, #0x0                   	// #0
  34:	ret
```

## prepare_push

### ref (queue.h)

```asm
0000000000000040 <ref_spsc_prepare_push>:
  40:	stp	x29, x30, [sp, #-64]!
  44:	stp	x24, x23, [sp, #16]
  48:	stp	x22, x21, [sp, #32]
  4c:	stp	x20, x19, [sp, #48]
  50:	mov	x29, sp
  54:	add	x20, x0, #0x4
  58:	ldr	w8, [x0, #8]
  5c:	ldr	w21, [x0]
  60:	ldar	w22, [x20]
  64:	sub	w23, w8, w21
  68:	add	w8, w23, w22
  6c:	cmp	w8, #0x0
  70:	b.gt	b8 <ref_spsc_prepare_push+0x78>
  74:	mov	x19, x0
  78:	nop
  7c:	nop
  80:	add	x1, x19, #0xc
  84:	mov	w0, #0x1                   	// #1
  88:	bl	0 <__aarch64_ldadd4_acq_rel>
			88: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  8c:	mov	x0, x20
  90:	mov	w1, w22
  94:	add	x24, x19, #0x4
  98:	bl	0 <atomic_wait>
			98: R_AARCH64_CALL26	atomic_wait
  9c:	ldar	w22, [x24]
  a0:	add	x1, x19, #0xc
  a4:	mov	w0, #0xffffffff            	// #-1
  a8:	bl	0 <__aarch64_ldadd4_acq_rel>
			a8: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  ac:	add	w8, w22, w23
  b0:	cmp	w8, #0x1
  b4:	b.lt	80 <ref_spsc_prepare_push+0x40>  // b.tstop
  b8:	mov	w0, w21
  bc:	ldp	x20, x19, [sp, #48]
  c0:	ldp	x22, x21, [sp, #32]
  c4:	ldp	x24, x23, [sp, #16]
  c8:	ldp	x29, x30, [sp], #64
  cc:	mov	x1, #0x0                   	// #0
  d0:	mov	x8, #0x0                   	// #0
  d4:	ret
  d8:	nop
  dc:	nop
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_prepare_push>:
   0:	stp	x29, x30, [sp, #-48]!
   4:	str	x21, [sp, #16]
   8:	stp	x20, x19, [sp, #32]
   c:	mov	x29, sp
  10:	add	x9, x0, #0x4
  14:	ldr	w8, [x0, #8]
  18:	ldar	w1, [x9]
  1c:	ldr	w20, [x0]
  20:	sub	w9, w1, w20
  24:	cmn	w9, w8
  28:	b.ne	4c <new_spsc_prepare_push+0x4c>  // b.any
  2c:	mov	x19, x0
  30:	neg	w21, w8
  34:	mov	x0, x19
  38:	bl	0 <new_spsc_prepare_push>
			38: R_AARCH64_CALL26	queuepp::SPSCQueueIndexer<float, 4ul>::WaitTail(unsigned int)
  3c:	mov	w1, w0
  40:	sub	w8, w0, w20
  44:	cmp	w8, w21
  48:	b.eq	34 <new_spsc_prepare_push+0x34>  // b.none
  4c:	mov	w0, w20
  50:	ldp	x20, x19, [sp, #32]
  54:	ldr	x21, [sp, #16]
  58:	ldp	x29, x30, [sp], #48
  5c:	mov	x1, #0x0                   	// #0
  60:	mov	x8, #0x0                   	// #0
  64:	mov	x9, #0x0                   	// #0
  68:	ret
```

## commit_push

### ref (queue.h)

```asm
00000000000000e0 <ref_spsc_commit_push>:
  e0:	stp	x29, x30, [sp, #-32]!
  e4:	stp	x20, x19, [sp, #16]
  e8:	mov	x29, sp
  ec:	mov	w0, #0x1                   	// #1
  f0:	mov	x19, x1
  f4:	add	x20, x1, #0xc
  f8:	bl	0 <__aarch64_ldadd4_acq_rel>
			f8: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  fc:	ldar	w8, [x20]
 100:	cbz	w8, 11c <ref_spsc_commit_push+0x3c>
 104:	mov	x0, x19
 108:	ldp	x20, x19, [sp, #16]
 10c:	ldp	x29, x30, [sp], #32
 110:	mov	x1, #0x0                   	// #0
 114:	mov	x8, #0x0                   	// #0
 118:	b	0 <atomic_wake_one>
			118: R_AARCH64_JUMP26	atomic_wake_one
 11c:	ldp	x20, x19, [sp, #16]
 120:	ldp	x29, x30, [sp], #32
 124:	mov	x1, #0x0                   	// #0
 128:	mov	x8, #0x0                   	// #0
 12c:	ret
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_commit_push>:
   0:	stp	x29, x30, [sp, #-32]!
   4:	str	x19, [sp, #16]
   8:	mov	x29, sp
   c:	mov	w0, #0x1                   	// #1
  10:	mov	x19, x1
  14:	bl	0 <__aarch64_ldadd4_acq_rel>
			14: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  18:	ldr	w8, [x19, #12]
  1c:	cbnz	w8, 34 <new_spsc_commit_push+0x34>
  20:	ldr	x19, [sp, #16]
  24:	ldp	x29, x30, [sp], #32
  28:	mov	x1, #0x0                   	// #0
  2c:	mov	x8, #0x0                   	// #0
  30:	ret
  34:	mov	x0, x19
  38:	ldr	x19, [sp, #16]
  3c:	ldp	x29, x30, [sp], #32
  40:	mov	x1, #0x0                   	// #0
  44:	mov	x8, #0x0                   	// #0
  48:	b	0 <new_spsc_commit_push>
			48: R_AARCH64_JUMP26	queuepp::SPSCQueueIndexer<float, 4ul>::WakeHead()
```

## try_prepare_consume

### ref (queue.h)

```asm
0000000000000130 <ref_spsc_try_prepare_consume>:
 130:	stp	x29, x30, [sp, #-16]!
 134:	mov	x29, sp
 138:	ldr	w8, [x0, #4]
 13c:	ldar	w9, [x0]
 140:	cmp	w9, w8
 144:	csinv	w0, w8, wzr, ne	// ne = any
 148:	ldp	x29, x30, [sp], #16
 14c:	mov	x8, #0x0                   	// #0
 150:	mov	x9, #0x0                   	// #0
 154:	ret
 158:	nop
 15c:	nop
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_try_prepare_consume>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	ldar	w8, [x0]
   c:	ldr	w9, [x0, #4]
  10:	cmp	w8, w9
  14:	csinv	w0, w9, wzr, ne	// ne = any
  18:	ldp	x29, x30, [sp], #16
  1c:	mov	x8, #0x0                   	// #0
  20:	mov	x9, #0x0                   	// #0
  24:	ret
```

## prepare_consume

### ref (queue.h)

```asm
0000000000000160 <ref_spsc_prepare_consume>:
 160:	stp	x29, x30, [sp, #-48]!
 164:	str	x21, [sp, #16]
 168:	stp	x20, x19, [sp, #32]
 16c:	mov	x29, sp
 170:	ldr	w19, [x0, #4]
 174:	ldar	w8, [x0]
 178:	cmp	w8, w19
 17c:	b.ne	1b4 <ref_spsc_prepare_consume+0x54>  // b.any
 180:	mov	x20, x0
 184:	add	x1, x20, #0xc
 188:	mov	w0, #0x1                   	// #1
 18c:	bl	0 <__aarch64_ldadd4_acq_rel>
			18c: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 190:	mov	x0, x20
 194:	mov	w1, w19
 198:	bl	0 <atomic_wait>
			198: R_AARCH64_CALL26	atomic_wait
 19c:	ldar	w21, [x20]
 1a0:	add	x1, x20, #0xc
 1a4:	mov	w0, #0xffffffff            	// #-1
 1a8:	bl	0 <__aarch64_ldadd4_acq_rel>
			1a8: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 1ac:	cmp	w21, w19
 1b0:	b.eq	184 <ref_spsc_prepare_consume+0x24>  // b.none
 1b4:	mov	w0, w19
 1b8:	ldp	x20, x19, [sp, #32]
 1bc:	ldr	x21, [sp, #16]
 1c0:	ldp	x29, x30, [sp], #48
 1c4:	mov	x1, #0x0                   	// #0
 1c8:	mov	x8, #0x0                   	// #0
 1cc:	ret
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_prepare_consume>:
   0:	stp	x29, x30, [sp, #-48]!
   4:	str	x21, [sp, #16]
   8:	stp	x20, x19, [sp, #32]
   c:	mov	x29, sp
  10:	ldar	w21, [x0]
  14:	ldr	w20, [x0, #4]
  18:	cmp	w21, w20
  1c:	b.ne	38 <new_spsc_prepare_consume+0x38>  // b.any
  20:	mov	x19, x0
  24:	mov	x0, x19
  28:	mov	w1, w21
  2c:	bl	0 <new_spsc_prepare_consume>
			2c: R_AARCH64_CALL26	queuepp::SPSCQueueIndexer<float, 4ul>::WaitHead(unsigned int)
  30:	cmp	w0, w21
  34:	b.eq	24 <new_spsc_prepare_consume+0x24>  // b.none
  38:	mov	w0, w20
  3c:	ldp	x20, x19, [sp, #32]
  40:	ldr	x21, [sp, #16]
  44:	ldp	x29, x30, [sp], #48
  48:	mov	x1, #0x0                   	// #0
  4c:	ret
```

## commit_consume

### ref (queue.h)

```asm
00000000000001d0 <ref_spsc_commit_consume>:
 1d0:	stp	x29, x30, [sp, #-32]!
 1d4:	stp	x20, x19, [sp, #16]
 1d8:	mov	x29, sp
 1dc:	mov	x19, x1
 1e0:	add	x20, x1, #0xc
 1e4:	add	x1, x1, #0x4
 1e8:	mov	w0, #0x1                   	// #1
 1ec:	bl	0 <__aarch64_ldadd4_acq_rel>
			1ec: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 1f0:	ldar	w8, [x20]
 1f4:	cbz	w8, 210 <ref_spsc_commit_consume+0x40>
 1f8:	add	x0, x19, #0x4
 1fc:	ldp	x20, x19, [sp, #16]
 200:	ldp	x29, x30, [sp], #32
 204:	mov	x1, #0x0                   	// #0
 208:	mov	x8, #0x0                   	// #0
 20c:	b	0 <atomic_wake_one>
			20c: R_AARCH64_JUMP26	atomic_wake_one
 210:	ldp	x20, x19, [sp, #16]
 214:	ldp	x29, x30, [sp], #32
 218:	mov	x1, #0x0                   	// #0
 21c:	mov	x8, #0x0                   	// #0
 220:	ret
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_commit_consume>:
   0:	stp	x29, x30, [sp, #-32]!
   4:	str	x19, [sp, #16]
   8:	mov	x29, sp
   c:	mov	x19, x1
  10:	add	x1, x1, #0x4
  14:	mov	w0, #0x1                   	// #1
  18:	bl	0 <__aarch64_ldadd4_acq_rel>
			18: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  1c:	ldr	w8, [x19, #12]
  20:	cbnz	w8, 38 <new_spsc_commit_consume+0x38>
  24:	ldr	x19, [sp, #16]
  28:	ldp	x29, x30, [sp], #32
  2c:	mov	x1, #0x0                   	// #0
  30:	mov	x8, #0x0                   	// #0
  34:	ret
  38:	mov	x0, x19
  3c:	ldr	x19, [sp, #16]
  40:	ldp	x29, x30, [sp], #32
  44:	mov	x1, #0x0                   	// #0
  48:	mov	x8, #0x0                   	// #0
  4c:	b	0 <new_spsc_commit_consume>
			4c: R_AARCH64_JUMP26	queuepp::SPSCQueueIndexer<float, 4ul>::WakeTail()
```

# NEVER_INLINE helpers (queuepp.hpp)

## WaitTail

```asm
0000000000000000 <queuepp::SPSCQueueIndexer<float, 4ul>::WaitTail(unsigned int)>:
   0:	stp	x29, x30, [sp, #-48]!
   4:	str	x21, [sp, #16]
   8:	stp	x20, x19, [sp, #32]
   c:	mov	x29, sp
  10:	mov	w19, w1
  14:	mov	x20, x0
  18:	add	x1, x0, #0xc
  1c:	mov	w0, #0x1                   	// #1
  20:	bl	0 <__aarch64_ldadd4_acq>
			20: R_AARCH64_CALL26	__aarch64_ldadd4_acq
  24:	add	x21, x20, #0x4
  28:	mov	w0, #0x62                  	// #98
  2c:	mov	x1, x21
  30:	mov	w2, #0x80                  	// #128
  34:	mov	w3, w19
  38:	mov	x4, xzr
  3c:	mov	x5, xzr
  40:	mov	w6, wzr
  44:	bl	0 <syscall>
			44: R_AARCH64_CALL26	syscall
  48:	ldar	w19, [x21]
  4c:	add	x1, x20, #0xc
  50:	mov	w0, #0xffffffff            	// #-1
  54:	bl	0 <__aarch64_ldadd4_relax>
			54: R_AARCH64_CALL26	__aarch64_ldadd4_relax
  58:	mov	w0, w19
  5c:	ldp	x20, x19, [sp, #32]
  60:	ldr	x21, [sp, #16]
  64:	ldp	x29, x30, [sp], #48
  68:	mov	x1, #0x0                   	// #0
  6c:	mov	x2, #0x0                   	// #0
  70:	mov	x3, #0x0                   	// #0
  74:	mov	x4, #0x0                   	// #0
  78:	mov	x5, #0x0                   	// #0
  7c:	mov	x6, #0x0                   	// #0
  80:	ret
```

## WakeTail

```asm
0000000000000000 <queuepp::SPSCQueueIndexer<float, 4ul>::WakeTail()>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	mov	x8, x0
   c:	mov	w0, #0x62                  	// #98
  10:	add	x1, x8, #0x4
  14:	mov	w2, #0x81                  	// #129
  18:	mov	w3, #0x1                   	// #1
  1c:	ldp	x29, x30, [sp], #16
  20:	mov	x8, #0x0                   	// #0
  24:	b	0 <syscall>
			24: R_AARCH64_JUMP26	syscall
```

## WaitHead

```asm
0000000000000000 <queuepp::SPSCQueueIndexer<float, 4ul>::WaitHead(unsigned int)>:
   0:	stp	x29, x30, [sp, #-32]!
   4:	stp	x20, x19, [sp, #16]
   8:	mov	x29, sp
   c:	mov	w19, w1
  10:	mov	x20, x0
  14:	add	x1, x0, #0xc
  18:	mov	w0, #0x1                   	// #1
  1c:	bl	0 <__aarch64_ldadd4_acq>
			1c: R_AARCH64_CALL26	__aarch64_ldadd4_acq
  20:	mov	w0, #0x62                  	// #98
  24:	mov	x1, x20
  28:	mov	w2, #0x80                  	// #128
  2c:	mov	w3, w19
  30:	mov	x4, xzr
  34:	mov	x5, xzr
  38:	mov	w6, wzr
  3c:	bl	0 <syscall>
			3c: R_AARCH64_CALL26	syscall
  40:	ldar	w19, [x20]
  44:	add	x1, x20, #0xc
  48:	mov	w0, #0xffffffff            	// #-1
  4c:	bl	0 <__aarch64_ldadd4_relax>
			4c: R_AARCH64_CALL26	__aarch64_ldadd4_relax
  50:	mov	w0, w19
  54:	ldp	x20, x19, [sp, #16]
  58:	ldp	x29, x30, [sp], #32
  5c:	mov	x1, #0x0                   	// #0
  60:	mov	x2, #0x0                   	// #0
  64:	mov	x3, #0x0                   	// #0
  68:	mov	x4, #0x0                   	// #0
  6c:	mov	x5, #0x0                   	// #0
  70:	mov	x6, #0x0                   	// #0
  74:	ret
```

## WakeHead

```asm
0000000000000000 <queuepp::SPSCQueueIndexer<float, 4ul>::WakeHead()>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	mov	x1, x0
   c:	mov	w0, #0x62                  	// #98
  10:	mov	w2, #0x81                  	// #129
  14:	mov	w3, #0x1                   	// #1
  18:	ldp	x29, x30, [sp], #16
  1c:	b	0 <syscall>
			1c: R_AARCH64_JUMP26	syscall
```

