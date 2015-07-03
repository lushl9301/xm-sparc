/*
 * $FILE: xm_def.h
 *
 * XM's SPARCv8 hardware configuration
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_XMDEF_H_
#define _XM_ARCH_XMDEF_H_

/* <track id="DOC_SYSCALL_TRAP_NR"> */
#define XM_HYPERCALL_TRAP 0xF0
#define XM_ASMHYPERCALL_TRAP 0xF1
/* </track id="DOC_SYSCALL_TRAP_NR"> */

#define XM_FPU_REG_NR (32+1)
#define NR_PPAG 4

#ifdef CONFIG_LEON3
#include __XM_INCFLD(arch/leon.h)
#else
#error Processor not valid
#endif

#define __ARCH_MEMCPY


#endif
