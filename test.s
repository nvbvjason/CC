	.file	"test.c"
	.text
	.globl	partial
	.section	.rodata
.LC0:
	.string	"Hello!"
	.section	.data.rel.local,"aw"
	.align 8
	.type	partial, @object
	.size	partial, 8
partial:
	.quad	.LC0
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.data.rel.local
	.align 8
	.type	msg.0, @object
	.size	msg.0, 8
msg.0:
	.quad	.LC0
	.ident	"GCC: (GNU) 15.2.1 20251112"
	.section	.note.GNU-stack,"",@progbits
