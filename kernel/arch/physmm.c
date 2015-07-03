/*
 * $FILE: physmm.c
 *
 * Physical memory manager 
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <physmm.h>
#include <arch/physmm.h>

xmAddress_t EnableByPassMmu(xmAddress_t addr, partition_t * p, struct physPage ** page){
    return addr;
}

inline void DisableByPassMmu(struct physPage * page){
}

