/*
 * $FILE: smp.c
 *
 * Symmetric multiprocessor support
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
#include <smp.h>
#include <spinlock.h>

void __VBOOT SetupSmp(void) {
    extern xm_u8_t SparcGetNoCpus(void);
    xm_s32_t cpu;
    
    SET_NRCPUS((SparcGetNoCpus()<xmcTab.hpv.noCpus)?SparcGetNoCpus():xmcTab.hpv.noCpus);
    for (cpu=GET_CPU_ID()+1; cpu<GET_NRCPUS(); cpu++)
        WakeUpCpu(cpu);
}
