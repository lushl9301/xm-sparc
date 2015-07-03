/*
 * $FILE: smp.h
 *
 * SMP related stuff
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_SMP_H_
#define _XM_ARCH_SMP_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <linkage.h>

#ifdef CONFIG_SMP
#define HALT_ALL_IPI_VECTOR XM_HALT_IPI_IRQ
#define SCHED_PENDING_IPI_VECTOR XM_SCHED_IPI_IRQ

#define NO_SHORTHAND_IPI 0x0
#define SELF_IPI 0x1
#define ALL_INC_SELF 0x2
#define ALL_EXC_SELF 0x3

extern void SendIpi(xm_u8_t dst, xm_u8_t dstShortHand, xm_u8_t vector);

#define SmpHaltAll() SendIpi(0, ALL_EXC_SELF, HALT_ALL_IPI_VECTOR)

extern void WakeUpCpu(xm_s32_t cpu);
#endif

extern xm_u32_t __GetCpuId(void);
extern xm_u32_t __GetCpuHwId(void);
extern void __SetCpuId(xm_u32_t);
extern void __SetCpuHwId(xm_u32_t);

#define GET_CPU_ID() __GetCpuId()
#define GET_CPU_HWID() __GetCpuHwId()
#define SET_CPU_ID(id) __SetCpuId(id)
#define SET_CPU_HWID(hwId) __SetCpuHwId(hwId)

#endif
