/*
 * $FILE: gen_offsets.h
 *
 * ASM offsets, this file only can be included from asm-offset.c
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef __GEN_OFFSETS_H_
#define _GEN_OFFSETS_H_
#ifndef _GENERATE_OFFSETS_
#error Do not include this file
#endif

#include <objects/commports.h>
#include <objects/console.h>
#include <objects/status.h>
#include <drivers/memblock.h>
#include <arch/atomic.h>
#include <kthread.h>
#include <sched.h>
#include <kdevice.h>
#include <logstream.h>
#include <physmm.h>

static inline void GenerateOffsets(void) {
    // localSched_t
    DEFINE(cKThread, offsetof(localSched_t, cKThread),);
    DEFINE(fpuOwner, offsetof(localSched_t, fpuOwner),);
    
    // kthread_t
    DEFINE(ctrl, offsetof(kThread_t, ctrl),);
    
    // struct __kthread
    DEFINE(flags, offsetof(struct __kThread, flags),);
    DEFINE(g, offsetof(struct __kThread, g),);
    DEFINE(kStack, offsetof(struct __kThread, kStack),);
    DEFINE(irqCpuCtxt, offsetof(struct __kThread, irqCpuCtxt),);
    
    // struct guest
    DEFINE(partCtrlTab, offsetof(struct guest, partCtrlTab),);
    DEFINE(kArch, offsetof(struct guest, kArch),);
    DEFINE(swTrap, offsetof(struct guest, swTrap),);

    // gctrl_t
    DEFINE(iFlags, offsetof(partitionControlTable_t, iFlags),);
    DEFINE(idPart, offsetof(partitionControlTable_t, id),);
    
    // xm_atomic_t
    DEFINE(val, offsetof(xmAtomic_t, val),);  
    
    // struct kthread_arch
    DEFINE(tbr, offsetof(struct kThreadArch, tbr),);
    DEFINE(fpuRegs, offsetof(struct kThreadArch, fpuRegs),);
    
    // struct  irqTabEntry
    DEFINE(handler, offsetof(struct irqTabEntry, handler), );
    DEFINE(data, offsetof(struct irqTabEntry, data), );
    
    // struct xmc hpv
    DEFINE(hpv, offsetof(struct xmc, hpv), );

    // struct xmcHpv cpuTab
    DEFINE(cpuTab, offsetof(struct xmcHpv, cpuTab), );

    //  struct cpu features
    DEFINE(features, offsetof(struct _cpu, features), );

    // struct cpuCtxt fields
    DEFINE(prev, offsetof(struct _cpuCtxt, prev), );
    DEFINE(psr, offsetof(struct _cpuCtxt, psr), );
    DEFINE(pc, offsetof(struct _cpuCtxt, pc), );
    DEFINE(nPc, offsetof(struct _cpuCtxt, nPc), );
    DEFINE(y, offsetof(struct _cpuCtxt, y), );
    DEFINE(g1, offsetof(struct _cpuCtxt, g1), );
    DEFINE(g2, offsetof(struct _cpuCtxt, g2), );
    DEFINE(g3, offsetof(struct _cpuCtxt, g3), );
    DEFINE(g4, offsetof(struct _cpuCtxt, g4), );
    DEFINE(g5, offsetof(struct _cpuCtxt, g5), );
    DEFINE(g6, offsetof(struct _cpuCtxt, g6), );
    DEFINE(g7, offsetof(struct _cpuCtxt, g7), );
    
    // sizeof
    DEFINE2(kthread_t,  sizeof(kThread_t), );
    DEFINE2(kthreadptr_t,  sizeof(kThread_t *), );
    DEFINE2(partition_t, sizeof(partition_t), );
    DEFINE2(struct_guest,  sizeof(struct guest), );
    DEFINE2(kdevice_t, sizeof(kDevice_t), );
    DEFINE2(struct_memblockdata, sizeof(struct memBlockData), );    
    DEFINE2(struct_console, sizeof(struct console), );
    DEFINE2(xmPartitionStatus_t, sizeof(xmPartitionStatus_t), );
    DEFINE2(struct_logstream, sizeof(struct logStream), );
    DEFINE2(union_channel, sizeof(union channel), );
    DEFINE2(struct_port, sizeof(struct port), );
    DEFINE2(struct_msg, sizeof(struct msg), );
    DEFINE2(struct_physpageptr, sizeof(struct physPage *), );
    DEFINE2(struct_physpage, sizeof(struct physPage), );
    DEFINE2(struct_scheddata, sizeof(struct schedData), );
    DEFINE2(cpuctxt, sizeof(cpuCtxt_t), );
}

#endif
