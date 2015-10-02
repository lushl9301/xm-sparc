/*
 * $FILE: processor.h
 *
 * Processor
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_PROCESSOR_H_
#define _XM_ARCH_PROCESSOR_H_

#define FLUSH_PAGE (0<<8)
#define FLUSH_SEGMENT (1<<8)
#define FLUSH_REGION (2<<8)
#define FLUSH_CONTEXT (3<<8)
#define FLUSH_ENTIRE (4<<8)

#ifdef _XM_KERNEL_

#include <arch/leon.h>

#ifndef __ASSEMBLY__

#ifdef CONFIG_LEON3
#define HwSaveSp(sp) \
    __asm__ __volatile__ ("mov %%sp, %0\n\t" : "=r" (sp))

#define HwSaveFp(fp) \
    __asm__ __volatile__ ("mov %%fp, %0\n\t" : "=r" (fp))

#define HwDisableFpu() do { \
    xm_u32_t __tmp1, __tmp2; \
    __asm__ __volatile__ ("rd %%psr, %0\n\t" \
                          "set "TO_STR(PSR_EF_BIT)", %1\n\t" \
                          "andn %0, %1, %0\n\t" \
                          "wr %0, %%psr\n\t" : "=r" (__tmp1), "=r" (__tmp2):); \
} while(0)

#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

static inline void FlushDCache(void) {
    xm_u32_t cCR;
    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
    cCR|=1<<22;
    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
}

static inline void FlushICache(void) {
    xm_u32_t cCR;
     __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
     cCR|=1<<21;
    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
}

/// == Flush I+D Cache
static inline void FlushCache(void) {
    xm_u32_t cCR;
    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
    cCR|=0x3<<21;
    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
}

// DCACHE|ICACHE = 0xf
#define DCACHE 0xc
#define ICACHE 0x3

static inline void SetCacheState(xm_u32_t cache) {
    xm_u32_t cCR;

    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
    cCR|=(cache&0xf)<<21;
    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");

}

#endif //CONFIG_LEON3

static inline void EnableMmu(void) {
    xmAddress_t mmuReg=LEON_MMU_CTRL_REG;
    xm_u32_t tmp=0;

    __asm__ __volatile__ ("lda [%1] %2, %0\n\t" \
			  :"=r" (tmp): "r" (mmuReg), "i" (LEON_MMU_ASI): "memory");
    tmp|=0x1;
    __asm__ __volatile__ ("sta %2, [%0] %1\n\t" \
			  :: "r" (mmuReg), "i" (LEON_MMU_ASI), "r" (tmp) : "memory");
}

static inline void SetMmuCtxt(xm_u8_t ctxt) {
    __asm__ __volatile__ ("sta %0, [%1] %2\n\t" :: "r"(ctxt), "r"(LEON_MMU_CTXT_REG), "i"(LEON_MMU_ASI): "memory");
}

#define LoadXmPageTable() SetMmuCtxt(0)
#define LoadPartitionPageTable(k) SetMmuCtxt(k->ctrl.g->kArch.mmuCtxt)

static inline xm_u8_t GetMmuCtxt(void) {
    xm_u8_t ctxt;
    __asm__ __volatile__ ("lda [%1] %2, %0\n\t" :"=r" (ctxt): "r"(LEON_MMU_CTXT_REG), "i"(LEON_MMU_ASI): "memory");
    return ctxt;
}

static inline void SetMmuCtxtPtr(xm_u32_t *ctxtPtr) {
    xmAddress_t cPtr=(xmAddress_t)ctxtPtr>>4;
    __asm__ __volatile__ ("sta %0, [%1] %2\n\t" :: "r"(cPtr), "r"(LEON_MMU_CPR_REG), "i"(LEON_MMU_ASI): "memory");
}

static inline xm_u32_t GetMmuFaultStatusReg(void) {
    xm_u32_t reg;
    __asm__ __volatile__ ("lda [%1] %2, %0\n\t" : "=r" (reg): "r"(LEON_MMU_FSTAT_REG), "i"(LEON_MMU_ASI): "memory");
    return reg;
}

static inline xm_u32_t GetMmuFaultAddressReg(void) {
    xm_u32_t reg;
    __asm__ __volatile__ ("lda [%1] %2, %0\n\t" : "=r" (reg): "r"(LEON_MMU_FADDR_REG), "i"(LEON_MMU_ASI): "memory");
    return reg;
}

static inline void FlushTlb(void) {
    FlushCache();
    __asm__ __volatile__("sta %%g0, [%0] %1\n\t"::"r"(FLUSH_ENTIRE), "i"(LEON_MMU_FLUSH):"memory");
}

static inline xm_u32_t GetPsr(void) {
    xm_u32_t psr;
    __asm__ __volatile__ ("rd %%psr, %0\n\t" : "=r" (psr):);
    return psr;
}

static inline void FlushTlbEntry(xmAddress_t addr) {
    xmAddress_t value=(addr&PAGE_MASK)|FLUSH_PAGE;
    __asm__ __volatile__("sta %%g0, [%0] %1\n\t"::"r"(value), "i"(LEON_MMU_FLUSH):"memory");
}

static inline void FlushTlbCtxt(void) {
    xmAddress_t value=FLUSH_CONTEXT;
    __asm__ __volatile__("sta %%g0, [%0] %1\n\t"::"r"(value), "i"(LEON_MMU_FLUSH):"memory");
}


#define FlushTlbGlobal() FlushTlb() //Just FlushTlb

#else

// same as above
#define FLUSH_ICACHE(_r0, _r1) \
    lda [%g0] LEON_CCR_BASE, _r0 ; \
    mov 1, _r1 ; \
    sll _r1, 21, _r1 ; \
    or _r0, _r1, _r0 ; \
    sta _r0, [%g0] LEON_CCR_BASE ;

#define FLUSH_DCACHE(_r0, _r1) \
    lda [%g0] LEON_CCR_BASE, _r0 ; \
    mov 1, _r1 ; \
    sll _r1, 22, _r1 ; \
    or _r0, _r1, _r0 ; \
    sta _r0, [%g0] LEON_CCR_BASE ;

#define FLUSH_CACHE(_r0, _r1) \
    FLUSH_ICACHE(_r0, _r1); \
    FLUSH_DCACHE(_r0, _r1);

#endif
#endif

#define CCR_DS_BIT (1<<23)
#define CCR_FD_BIT (1<<22) // flush DCache
#define CCR_FI_BIT (1<<21) // flush ICache
#define CCR_IB_BIT (1<<16)
#define CCR_DCS_MASK (0x3<<2) // #define DCACHE 0xc
#define CCR_ICS_MASK (0x3<<0) // #define ICACHE 0x3

#define PSR_IMPL_MASK 0xf0000000
#define PSR_VER_MASK 0x0f000000
#define PSR_ICC_MASK 0x00f00000 // Interger cond codes
#define PSR_EC_BIT 0x00002000 // Enable coprocessor
#define PSR_EF_BIT 0x00001000 // Enable floating point
#define PSR_PIL_MASK 0x00000f00 // Proc interrupt level
#define PSR_S_BIT 0x00000080 // Supervisor
#define PSR_PS_BIT 0x00000040 // Previous supervisor
#define PSR_ET_BIT 0x00000020 // Enable trap
#define PSR_CWP_MASK 0x0000001f // Current window pointer

#endif
