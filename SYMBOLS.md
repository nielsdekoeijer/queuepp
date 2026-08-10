# Symbol Comparison: queue.h (ref) vs queuepp.hpp (new)

## try_prepare_push

### ref (queue.h)

```asm
0000000000000000 <ref_spsc_try_prepare_push>:
   0:	stp	x29, x30, [sp, #-16]!
   4:	mov	x29, sp
   8:	ldr	w8, [x0, #8]
   c:	ldr	w9, [x0]
  10:	add	x10, x0, #0x4
  14:	ldar	w10, [x10]
  18:	sub	w8, w8, w9
  1c:	add	w8, w8, w10
  20:	cmp	w8, #0x1
  24:	csinv	w0, w9, wzr, ge	// ge = tcont
  28:	ldp	x29, x30, [sp], #16
  2c:	mov	x8, #0x0                   	// #0
  30:	mov	x9, #0x0                   	// #0
  34:	mov	x10, #0x0                   	// #0
  38:	ret
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
000000000000003c <ref_spsc_prepare_push>:
  3c:	stp	x29, x30, [sp, #-64]!
  40:	stp	x24, x23, [sp, #16]
  44:	stp	x22, x21, [sp, #32]
  48:	stp	x20, x19, [sp, #48]
  4c:	mov	x29, sp
  50:	ldr	w8, [x0, #8]
  54:	ldr	w20, [x0]
  58:	add	x21, x0, #0x4
  5c:	ldar	w22, [x21]
  60:	sub	w23, w8, w20
  64:	add	w8, w23, w22
  68:	cmp	w8, #0x0
  6c:	b.gt	ac <ref_spsc_prepare_push+0x70>
  70:	mov	x19, x0
  74:	add	x1, x19, #0xc
  78:	mov	w0, #0x1                   	// #1
  7c:	bl	0 <__aarch64_ldadd4_acq_rel>
			7c: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  80:	mov	x0, x21
  84:	mov	w1, w22
  88:	add	x24, x19, #0x4
  8c:	bl	0 <atomic_wait>
			8c: R_AARCH64_CALL26	atomic_wait
  90:	ldar	w22, [x24]
  94:	add	x1, x19, #0xc
  98:	mov	w0, #0xffffffff            	// #-1
  9c:	bl	0 <__aarch64_ldadd4_acq_rel>
			9c: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  a0:	add	w8, w22, w23
  a4:	cmp	w8, #0x1
  a8:	b.lt	74 <ref_spsc_prepare_push+0x38>  // b.tstop
  ac:	mov	w0, w20
  b0:	ldp	x20, x19, [sp, #48]
  b4:	ldp	x22, x21, [sp, #32]
  b8:	ldp	x24, x23, [sp, #16]
  bc:	ldp	x29, x30, [sp], #64
  c0:	mov	x1, #0x0                   	// #0
  c4:	mov	x8, #0x0                   	// #0
  c8:	ret
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
  3c:	sub	w8, w0, w20
  40:	mov	w1, w0
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
00000000000000cc <ref_spsc_commit_push>:
  cc:	stp	x29, x30, [sp, #-32]!
  d0:	stp	x20, x19, [sp, #16]
  d4:	mov	x29, sp
  d8:	mov	w0, #0x1                   	// #1
  dc:	mov	x19, x1
  e0:	add	x20, x1, #0xc
  e4:	bl	0 <__aarch64_ldadd4_acq_rel>
			e4: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
  e8:	ldar	w8, [x20]
  ec:	cbz	w8, 108 <ref_spsc_commit_push+0x3c>
  f0:	mov	x0, x19
  f4:	ldp	x20, x19, [sp, #16]
  f8:	ldp	x29, x30, [sp], #32
  fc:	mov	x1, #0x0                   	// #0
 100:	mov	x8, #0x0                   	// #0
 104:	b	0 <atomic_wake_one>
			104: R_AARCH64_JUMP26	atomic_wake_one
 108:	ldp	x20, x19, [sp, #16]
 10c:	ldp	x29, x30, [sp], #32
 110:	mov	x1, #0x0                   	// #0
 114:	mov	x8, #0x0                   	// #0
 118:	ret
```

### new (queuepp.hpp)

```asm
0000000000000000 <new_spsc_commit_push>:
   0:	stp	x29, x30, [sp, #-32]!
   4:	str	x19, [sp, #16]
   8:	mov	x29, sp
   c:	mov	w0, #0x1                   	// #1
  10:	mov	x19, x1
  14:	bl	0 <__aarch64_ldadd4_rel>
			14: R_AARCH64_CALL26	__aarch64_ldadd4_rel
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
000000000000011c <ref_spsc_try_prepare_consume>:
 11c:	stp	x29, x30, [sp, #-16]!
 120:	mov	x29, sp
 124:	ldr	w8, [x0, #4]
 128:	ldar	w9, [x0]
 12c:	cmp	w9, w8
 130:	csinv	w0, w8, wzr, ne	// ne = any
 134:	ldp	x29, x30, [sp], #16
 138:	mov	x8, #0x0                   	// #0
 13c:	mov	x9, #0x0                   	// #0
 140:	ret
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
0000000000000144 <ref_spsc_prepare_consume>:
 144:	stp	x29, x30, [sp, #-48]!
 148:	str	x21, [sp, #16]
 14c:	stp	x20, x19, [sp, #32]
 150:	mov	x29, sp
 154:	ldr	w19, [x0, #4]
 158:	ldar	w8, [x0]
 15c:	cmp	w8, w19
 160:	b.ne	198 <ref_spsc_prepare_consume+0x54>  // b.any
 164:	mov	x20, x0
 168:	add	x1, x20, #0xc
 16c:	mov	w0, #0x1                   	// #1
 170:	bl	0 <__aarch64_ldadd4_acq_rel>
			170: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 174:	mov	x0, x20
 178:	mov	w1, w19
 17c:	bl	0 <atomic_wait>
			17c: R_AARCH64_CALL26	atomic_wait
 180:	ldar	w21, [x20]
 184:	add	x1, x20, #0xc
 188:	mov	w0, #0xffffffff            	// #-1
 18c:	bl	0 <__aarch64_ldadd4_acq_rel>
			18c: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 190:	cmp	w21, w19
 194:	b.eq	168 <ref_spsc_prepare_consume+0x24>  // b.none
 198:	mov	w0, w19
 19c:	ldp	x20, x19, [sp, #32]
 1a0:	ldr	x21, [sp, #16]
 1a4:	ldp	x29, x30, [sp], #48
 1a8:	mov	x1, #0x0                   	// #0
 1ac:	mov	x8, #0x0                   	// #0
 1b0:	ret
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
00000000000001b4 <ref_spsc_commit_consume>:
 1b4:	stp	x29, x30, [sp, #-32]!
 1b8:	stp	x20, x19, [sp, #16]
 1bc:	mov	x29, sp
 1c0:	mov	x19, x1
 1c4:	add	x20, x1, #0xc
 1c8:	add	x1, x1, #0x4
 1cc:	mov	w0, #0x1                   	// #1
 1d0:	bl	0 <__aarch64_ldadd4_acq_rel>
			1d0: R_AARCH64_CALL26	__aarch64_ldadd4_acq_rel
 1d4:	ldar	w8, [x20]
 1d8:	cbz	w8, 1f4 <ref_spsc_commit_consume+0x40>
 1dc:	add	x0, x19, #0x4
 1e0:	ldp	x20, x19, [sp, #16]
 1e4:	ldp	x29, x30, [sp], #32
 1e8:	mov	x1, #0x0                   	// #0
 1ec:	mov	x8, #0x0                   	// #0
 1f0:	b	0 <atomic_wake_one>
			1f0: R_AARCH64_JUMP26	atomic_wake_one
 1f4:	ldp	x20, x19, [sp, #16]
 1f8:	ldp	x29, x30, [sp], #32
 1fc:	mov	x1, #0x0                   	// #0
 200:	mov	x8, #0x0                   	// #0
 204:	ret
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
  18:	bl	0 <__aarch64_ldadd4_rel>
			18: R_AARCH64_CALL26	__aarch64_ldadd4_rel
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

