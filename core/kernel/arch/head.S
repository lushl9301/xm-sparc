/*
 * $FILE: head.S
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <linkage.h>
#include <arch/asm_offsets.h>
#include <arch/irqs.h>
#include <arch/processor.h>
#include <arch/xm_def.h>
#include <arch/leon.h>

#define WIM_INIT  2

.macro WR_DELAY
	nop
	nop
	nop
.endm

/*
(16MB)
-----------------
4096 4KPAGES
1 L1
1 L2
64 L3
--------------
*/

#define NO_PTDL2_XMVMAP 1
#define NO_PTDL3_XMVMAP 64

.section .boot.text, "ax"
ASM_ALIGN
ENTRY(start)
ENTRY(_start)
#if defined(CONFIG_MMU)
	/* Disable paging */
	set LEON_MMU_CTRL_REG, %g1
	lda [%g1] LEON_MMU_ASI, %g2
	andn %g2, 0x1, %g2
	sta %g2, [%g1] LEON_MMU_ASI
	/* lda [0x0] to g2 */
	/* and write last bit back to leon_mmu_ctrl-reg */
#endif
	/* Enabling Cache and Flushing it */
	set CCR_DCS_MASK|CCR_ICS_MASK, %g1
#ifdef CONFIG_CACHE_SNOOP
	set CCR_DS_BIT, %g2
	or %g1, %g2, %g1
#endif
#ifdef CONFIG_CACHE_IBURST_FETCH
	set CCR_IB_BIT, %g2
	or %g1, %g2, %g1
#endif
	flush
#ifdef CONFIG_LEON3
	sta %g1, [%g0] LEON_CCR_BASE
	/* Cache control register; similar to LEON_MMU_CTRL_REG */
#endif

	/* MEMORY CONTROLLER CONFIGURATION MISS -- MIG */
#ifdef CONFIG_LEON3
	/* Set up the memory */
	set LEON_MEMORY_CFG_BASE, %g1
	set IOP_MASK, %g3
	ld [%g1], %g2
	or %g2, %g3, %g2
	st %g2, [%g1]
	/* st MEMORY_CFG_BASE as 0x8008000 */
#endif
#ifndef CONFIG_MMU
	set LEON_MEMORY_WPR_BASE, %g1
	st %g0, [%g1]
	add %g1, 4, %g1
	st %g0, [%g1]
#endif

#ifdef CONFIG_SMP
	rd %asr17, %g1 /* Processor configuration register */
	sra %g1, 0x1c, %g1 /* shift right 28, get PI processor ID */
	cmp %g1, 0
	bne 2f
	nop
	/* if this is master processor then go on; else goto 2 */
#endif

	set _VIRT2PHYS(_sbss), %o0
	set _VIRT2PHYS(_ebss), %o1
1:	stb %g0, [%o0]
	add %o0, 0x1, %o0
	subcc %o0, %o1, %g0 /* write to g0 means only care about flags */
	bl 1b
	nop
	/* memory clear from sbss to ebss */

	/* Copy the cdata section to the data section */
	set _VIRT2PHYS(_sdata), %o0
	set _VIRT2PHYS(_edata), %o1
	set _VIRT2PHYS(_scdata), %o2
	set _VIRT2PHYS(_cpdata), %o3

	/* Check if data has already been backed up */
	ld [%o3], %o4
	cmp %o4, %g0
	bne 1f
	/* if the mem to be copied is ZERO, then copy */
	mov 1, %o4
	st %o4, [%o3]

	/* TODO this loop-3 is almost similiar as loop-1 */
3:	ldub [%o0], %o3
	stb %o3, [%o2]
	add %o0, 0x1, %o0
	subcc %o0, %o1, %g0
	bl 3b
	add %o2, 0x1, %o2

	b 2f
	nop
	/* else */
1:	ldub [%o2], %o3
	stb %o3, [%o0]
	add %o0, 0x1, %o0
	subcc %o0, %o1, %g0
	bl 1b
	add %o2, 0x1, %o2

2:
	/* Clear general registers */
	clr %g1
	clr %g2
	clr %g3
	clr %g4
	clr %g5
	clr %g6
	clr %g7

	/* Set up window registers */
	mov %g0, %wim
	WR_DELAY

	mov CONFIG_REGISTER_WINDOWS, %g1 /* 8 register windows */
	save /* related to current window pointer */
1:
	clr %i0
	clr %i1
	clr %i2
	clr %i3
	clr %i4
	clr %i5
	clr %i6
	clr %i7
	clr %l0
	clr %l1
	clr %l2
	clr %l3
	clr %l4
	clr %l5
	clr %l6
	clr %l7
	subcc %g1, 1, %g1 /* clear all register windows */
	bnz,a   1b /* Instructions that deal with alternate space; hence register windows */
	save

	/* Set up PSR, WIM, TBR and Y registers  */
	mov WIM_INIT, %wim
	mov (PSR_PIL_MASK|PSR_ET_BIT|PSR_S_BIT|PSR_EF_BIT), %psr
	WR_DELAY
	mov %g0, %y
	/* Multiply/Divide Register (Y) contains the most significant word of */ 
	/* the double-precision product of an integer multiplication */

#ifdef CONFIG_MMU
#ifdef CONFIG_SMP
	rd %asr17, %g1
	sra %g1, 0x1c, %g1
	cmp %g1, 0 /* if not master processor then jump */
	bne 113f
	nop
#endif

	/* Fill ptdL3 with XM_VMAPSIZE/PAGE_SIZE pages */
	set ((CONFIG_XM_LOAD_ADDR&PAGE_MASK)>>4)|_PG_ARCH_RW|_PG_ARCH_CACHE|_PG_ARCH_PTE_PRESENT, %o0
	set _VIRT2PHYS(_ptdL3), %o1
	set NO_PTDL3_XMVMAP*PTDL3ENTRIES*4, %o3
	add %o1, %o3, %o2
	/* o2 stores end of ptdL3 */

1:	st %o0, [%o1]
	add %o1, 0x4, %o1
	subcc %o1, %o2, %g0
	bl 1b
	/* set page table with flag o0 */
	
	add %o0, (PAGE_SIZE>>4), %o0
	set _VIRT2PHYS(_ptdL3), %o0
	srl %o0, 4, %o0
	or %o0, _PG_ARCH_PTD_PRESENT, %o0

	/* Fill ptdL2 with the correspondent ptdL3 entries */
	set _VIRT2PHYS(_ptdL2), %o1
	set NO_PTDL2_XMVMAP*PTDL2ENTRIES*4, %o3
	add %o1, %o3, %o2

1:	st %o0, [%o1]
	add %o1, 0x4, %o1
	subcc %o1, %o2, %g0
	bl 1b
	/* for loop to clear from o1 to o2 */
	add %o0, 16, %o0

	/* Fill ptdL1 with the correspondent ptdL2 entries (at XM_OFFSET) */
	set _VIRT2PHYS(_ptdL2), %o0
	srl %o0, 4, %o0
	or %o0, _PG_ARCH_PTD_PRESENT, %o0
	set _VIRT2PHYS(_ptdL1), %o1
	set NO_PTDL2_XMVMAP*4, %o3
	set (CONFIG_XM_OFFSET>>24)*4, %o4
	add %o1, %o4, %o1

	add %o1, %o3, %o2

1:	st %o0, [%o1]
	add %o1, 0x4, %o1
	subcc %o1, %o2, %g0
	bl 1b
	add %o0, 16, %o0

	/* Fill ptdL1 with the correspondent ptdL2 entries (at XM_LOAD_ADDR) */
	set _VIRT2PHYS(_ptdL2), %o0
	srl %o0, 4, %o0
	or %o0, _PG_ARCH_PTD_PRESENT, %o0

	set _VIRT2PHYS(_ptdL1), %o1
	set NO_PTDL2_XMVMAP*4, %o3
	set (CONFIG_XM_LOAD_ADDR>>24)*4, %o4
	add %o1, %o4, %o1

	add %o1, %o3, %o2

1:	st %o0, [%o1]
	add %o1, 0x4, %o1
	subcc %o1, %o2, %g0
	bl 1b
	add %o0, 16, %o0

	/* Fill the context table with ptdL1 */
	set _VIRT2PHYS(_ptdL1), %o0
	srl %o0, 4, %o0
	or %o0, _PG_ARCH_PTD_PRESENT, %o0

	set _VIRT2PHYS(contextTab), %o1
	st %o0, [%o1]

113:
	/* Load MMU CTXT PTR <- (contextTab>>4) */
	set _VIRT2PHYS(contextTab), %o1
	srl %o1, 4, %o1

	set LEON_MMU_CPR_REG, %g1
	sta %o1, [%g1] LEON_MMU_ASI /*store into [%g1]

	/* Load MMU CTXT <- 0 */
	set LEON_MMU_CTXT_REG, %g1
	sta %g0, [%g1] LEON_MMU_ASI

	/* Enable MMU CTRL_REG <- 1 */
	set LEON_MMU_CTRL_REG, %g1
	lda [%g1] LEON_MMU_ASI, %g2
	or %g2, 0x1, %g2
	sta %g2, [%g1] LEON_MMU_ASI
#endif
	/* hi extract high 22 bit */
	sethi %hi(StartXM), %g1
	jmpl %g1 + %lo(StartXM), %g0 /* this is same as jmp %g1 + %lo(StartXM) */
	nop

_Reset2:
#if defined(CONFIG_MMU)
	/* Disable paging */
	set LEON_MMU_CTRL_REG, %g1
	lda [%g1] LEON_MMU_ASI, %g2
	andn %g2, 0x1, %g2
	sta %g2, [%g1] LEON_MMU_ASI
#endif
	jmp %o0
	nop

.text

ENTRY(StartXM)
	set trapTable, %g1
	/* TODO wr %g0, reg_or_imm, %tbr according to manual */
	wr %g1, %tbr


	/* Setting up a kernel stack */
#ifdef CONFIG_SMP
	rd %asr17, %o0
	sra %o0, 0x1c, %o0
#else
	mov %g0, %o0
#endif
	set CONFIG_KSTACK_SIZE, %g3
	add %o0, 1, %g1

	set __idle_kthread, %fp
1:	dec %g1
	cmp %g1, 0
	bg 1b
	/* while cCpu != 0; cCpu--; */
	add %fp, %g3, %fp

	sub %fp, %g3, %o1
	sub %fp, MIN_STACK_FRAME, %sp
#ifdef CONFIG_SMP
	cmp %o0, 0
	bne 1f
#endif
	/* TODO why again? */
	sub %fp, MIN_STACK_FRAME, %sp

	/* calling our first C function */
	call Setup
	nop
#ifdef CONFIG_SMP
1:
	/* each core will call this by themself */
	call InitSecondaryCpu
	nop
#endif

ENTRY(__HaltSystem)
1:	b 1b
	nop
	/* Reset the processor */

ENTRY(ArchStartupGuest)
	ldd [%sp], %o0
	jmpl %g4, %g0
	add %sp, 8, %sp

ENTRY(_Reset)
#ifdef CONFIG_MMU
	/* Load MMU CTXT <- 0 */
	set LEON_MMU_CTXT_REG, %g1
	sta %g0, [%g1] LEON_MMU_ASI
#endif
	set _Reset2, %g1
	jmp %g1
	/* reset CPUs */
	nop

.section ".bss"

#ifdef CONFIG_MMU
.align 1024
ENTRY(contextTab)
	.zero CTXTTABSIZE

.align 1024
ENTRY(_pgTables)
ENTRY(_ptdL1)
	.zero PTDL1SIZE


/*
(16MB)
-----------------
4096 4KPAGES
1 L1
1 L2
64 L3
--------------
*/

.align 256
ENTRY(_ptdL2)
	.zero NO_PTDL2_XMVMAP*PTDL2SIZE

.align 256
ENTRY(_ptdL3)
	.zero NO_PTDL3_XMVMAP*PTDL3SIZE

#endif

ASM_ALIGN
__idle_kthread:
	.zero CONFIG_KSTACK_SIZE*CONFIG_NO_CPUS
ASM_ALIGN
__idle_kthread_top:

/*
.align 128*1024
.global ioMmuApv
ioMmuApv:
.zero 128*1024
*/
.previous
