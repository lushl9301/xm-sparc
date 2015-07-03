/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel (arch dependent part)
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
#include <stdc.h>
#include <physmm.h>

void __VBOOT SetupArchLocal(xm_s32_t cpuid) {
}

void __VBOOT EarlySetupArchCommon(void) {
#ifdef CONFIG_AUDIT_EVENTS
    extern xm_u8_t auditAsmHCall[NR_ASM_HYPERCALLS];
#endif
    extern void EarlySetupCpu(void);
    /* There is at least one processor in the system */
    SET_NRCPUS(1);
    EarlySetupCpu();
#ifdef CONFIG_AUDIT_EVENTS
    memset(auditAsmHCall, 0, NR_ASM_HYPERCALLS*sizeof(xm_u8_t));
#endif
}

void __VBOOT SetupArchCommon(void) {
}
