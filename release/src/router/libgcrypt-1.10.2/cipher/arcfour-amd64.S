/*
** RC4 implementation optimized for AMD64.
**
** Author: Marc Bevand <bevand_m (at) epita.fr>
** Licence: I hereby disclaim the copyright on this code and place it
** in the public domain.
**
** The throughput achieved by this code is about 320 MBytes/sec, on
** a 1.8 GHz AMD Opteron (rev C0) processor.
**
** 2013/12/20 <jussi.kivilinna@iki.fi>:
**  - Integrated to libgcrypt
**  - 4.18 cycles/byte on Intel i5-4570
*/

#ifdef __x86_64__
#include <config.h>
#if defined(USE_ARCFOUR) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))

#include "asm-common-amd64.h"

.text
.align 16
.globl _gcry_arcfour_amd64
ELF(.type _gcry_arcfour_amd64,@function)
_gcry_arcfour_amd64:
	CFI_STARTPROC()
	ENTER_SYSV_FUNC_PARAMS_0_4
	push	%rbp
	CFI_PUSH(%rbp)
	push	%rbx
	CFI_PUSH(%rbx)
	mov	%rdi,		%rbp	# key = ARG(key)
	mov	%rsi,		%rbx	# rbx = ARG(len)
	mov	%rdx,		%rsi	# in = ARG(in)
	mov	%rcx,		%rdi	# out = ARG(out)
	mov	(4*256)(%rbp),	%ecx	# x = key->x
	mov	(4*256+4)(%rbp),%edx	# y = key->y
	inc	%rcx			# x++
	and	$255,		%rcx	# x &= 0xff
	lea	-8(%rbx,%rsi),	%rbx	# rbx = in+len-8
	mov	%rbx,		%r9	# tmp = in+len-8
	mov	(%rbp,%rcx,4),	%eax	# tx = d[x]
	cmp	%rsi,		%rbx	# cmp in with in+len-8
	jl	.Lend			# jump if (in+len-8 < in)

.Lstart:
	add	$8,		%rsi		# increment in
	add	$8,		%rdi		# increment out

	# generate the next 8 bytes of the rc4 stream into %r8
	mov	$8,		%r11		# byte counter
1:	add	%al,		%dl		# y += tx
	mov	(%rbp,%rdx,4),	%ebx		# ty = d[y]
	mov	%ebx,		(%rbp,%rcx,4)	# d[x] = ty
	add	%al,		%bl		# val = ty + tx
	mov	%eax,		(%rbp,%rdx,4)	# d[y] = tx
	inc	%cl				# x++		(NEXT ROUND)
	mov	(%rbp,%rcx,4),	%eax		# tx = d[x]	(NEXT ROUND)
	shl	$8,		%r8
	movb	(%rbp,%rbx,4),	%r8b		# val = d[val]
	dec	%r11b
	jnz 1b

	# xor 8 bytes
	bswap	%r8
	xor	-8(%rsi),	%r8
	cmp	%r9,		%rsi		# cmp in+len-8 with in
	mov	%r8,		-8(%rdi)
	jle	.Lstart				# jump if (in <= in+len-8)

.Lend:
	add	$8,		%r9		# tmp = in+len

	# handle the last bytes, one by one
1:	cmp	%rsi,		%r9		# cmp in with in+len
	jle	.Lfinished			# jump if (in+len <= in)
	add	%al,		%dl		# y += tx
	mov	(%rbp,%rdx,4),	%ebx		# ty = d[y]
	mov	%ebx,		(%rbp,%rcx,4)	# d[x] = ty
	add	%al,		%bl		# val = ty + tx
	mov	%eax,		(%rbp,%rdx,4)	# d[y] = tx
	inc	%cl				# x++		(NEXT ROUND)
	mov	(%rbp,%rcx,4),	%eax		# tx = d[x]	(NEXT ROUND)
	movb	(%rbp,%rbx,4),	%r8b		# val = d[val]
	xor	(%rsi),		%r8b		# xor 1 byte
	movb	%r8b,		(%rdi)
	inc	%rsi				# in++
	inc	%rdi				# out++
	jmp 1b

.Lfinished:
	dec	%rcx				# x--
	movb	%cl,		(4*256)(%rbp)	# key->y = y
	movb	%dl,		(4*256+4)(%rbp)	# key->x = x
	pop	%rbx
	CFI_POP(%rbx)
	pop	%rbp
	CFI_POP(%rbp)
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC()
.L__gcry_arcfour_amd64_end:
ELF(.size _gcry_arcfour_amd64,.L__gcry_arcfour_amd64_end-_gcry_arcfour_amd64)

#endif
#endif
