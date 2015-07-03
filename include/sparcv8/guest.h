/*
 * $FILE: guest.h
 *
 * Guest shared info
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_GUEST_H_
#define _XM_ARCH_GUEST_H_

#include __XM_INCFLD(arch/atomic.h)
#include __XM_INCFLD(arch/processor.h)

// XXX: this structure is visible from the guest
/*  <track id="doc-Partition-Control-Table-Arch"> */
struct pctArch {
    xmAddress_t tbr;
#ifdef CONFIG_MMU
    volatile xmAddress_t ptdL1;
#define _ARCH_PTDL1_REG ptdL1
    volatile xm_u32_t faultStatusReg;
    volatile xm_u32_t faultAddressReg;
#endif
};
/*  </track id="doc-Partition-Control-Table-Arch"> */

#endif
