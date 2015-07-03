/*
 * $FILE: processor.c
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

#include <assert.h>
#include <boot.h>
#include <linkage.h>
#include <kdevice.h>
#include <processor.h>
#include <physmm.h>
#include <sched.h>
#include <spinlock.h>
#include <stdc.h>
#include <xmconf.h>

#include <arch/xm_def.h>

RESERVE_IOPORTS(LEON_MEMORY_CFG_BASE, 3);
RESERVE_IOPORTS(LEON_MEMORY_WPR_BASE, 2);

#define SCAR_REG 0x4

xm_u32_t GetCpuKhz(void) {
#ifdef CONFIG_LEON3
    xm_u32_t pFreq=(LoadIoReg(LEON_TIMER_CFG_BASE+SCAR_REG)+1)*1000;

    if (xmcTab.hpv.cpuTab[GET_CPU_ID()].freq==XM_CPUFREQ_AUTO)
        return pFreq;

    if (xmcTab.hpv.cpuTab[GET_CPU_ID()].freq!=pFreq)
        PWARN("XMC freq (%dKhz) mismatches hw detected (%dKhz)\n", xmcTab.hpv.cpuTab[GET_CPU_ID()].freq, pFreq);
#endif
    return xmcTab.hpv.cpuTab[GET_CPU_ID()].freq;
}

xm_u32_t __GetCpuId(void) {
#ifdef CONFIG_SMP
    xm_u32_t cpuId;
    __asm__ __volatile__ ("rd %%asr17, %0\n\t" : "=r" (cpuId):);
    return (cpuId>>28);
#else
    return 0;
#endif
}

xm_u32_t __GetCpuHwId(void) {
#ifdef CONFIG_SMP
    xm_u32_t cpuId;
    __asm__ __volatile__ ("rd %%asr17, %0\n\t" : "=r" (cpuId):);
    return (cpuId>>28);
#else
    return 0;
#endif
}

void __SetCpuId(xm_u32_t id) {
}

void __SetCpuHwId(xm_u32_t hwId) {
}

void __VBOOT SetupCpu(void) {
    xm_u32_t cache=0;
#ifdef CONFIG_ENABLE_CACHE
    cache=DCACHE|ICACHE;
#endif
    SetCacheState(cache);

#if !defined(CONFIG_MMU) && defined(CONFIG_WA_PROTECT_MMAP_PREG_WPOINT)
#define WA_3G_AND 0xC0000000
#define WA_3G_XOR 0x80000000
    if ((xmcTab.hpv.cpuTab[0].features&XM_CPU_FEATURE_WA1)) {
	__asm__ __volatile__ ("set "TO_STR(WA_3G_AND)", %%g1\n\t" \
			      "wr %%g1, %%asr25\n\t" \
			      "set "TO_STR(WA_3G_XOR)", %%g1\n\t" \
			      "wr %%g1, %%asr24\n\t" \
			      "nop;nop;nop\n\t"::: "g1");
    }
    
#endif
}

void __VBOOT EarlySetupCpu(void) {
    cpuKhz=GetCpuKhz();
}
