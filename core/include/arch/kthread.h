/*
 * $FILE: kthread.h
 *
 * Kernel, Guest or Layer0 thread
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_KTHREAD_H_
#define _XM_ARCH_KTHREAD_H_

#include <irqs.h>
#include <arch/processor.h>
#include <arch/xm_def.h>
#include <arch/atomic.h>

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

struct kThreadArch {
#if defined(CONFIG_MPU)
    xm_u32_t wpReg;
#else
    xmAddress_t ptdL1;
    xmAddress_t xPtdL1;
    xm_u8_t mmuCtxt;
#endif
    xm_u32_t tbr;
    // Floating point 
    // XM does not use fpu
    xm_u32_t fpuRegs[XM_FPU_REG_NR] ALIGNED_C;
};

#endif
