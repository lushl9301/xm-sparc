
******
## objectTab

### Declaration

	//file core/kernel/objdir.c
    struct xmcRswInfo *xmcRswInfo;const struct object *objectTab[OBJ_NO_CLASSES]={[0 ... OBJ_NO_CLASSES-1] = 0};

### Description

An array of pointers of XM objects. Objects here are sampling ports, queuing ports, health monitor, console, memory, etc.. It is easy to invoke an operation of a certain object by simply calling ```objectTab[OBJ_CLASS_CONSOLE]->Read(...)```.

### Initialization

Initialized by assigned with object's address.

### Functions

1. ReadObjectSys & WriteObjectSys & SeekObjectSys & CtrlObjectSys

	Get object's offset on the array from object descriptor. Check if the object is not NULL and has the corresponding function.

2. OBJDESC_GET_CLASS

    // Object descriptor:
    // VALIDITY  | CLASS| vCPUID | PARTITIONID | ID
    //     1     |   7  |   4    |      10     |  10
    return ((oD>>24)&OBJDESC_CLASS_MASK)

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description

The number of real CPUs. Initially, it is set to 0. And it is updated by calling ```SET_NRCPUS((SparcGetNoCpus()<xmcTab.hpv.noCpus)?SparcGetNoCpus():xmcTab.hpv.noCpus)```, where ```SparcGetNoCpus()``` is implemented by:
```c
xm_u8_t SparcGetNoCpus(void) {
    return (LoadIoReg(GET_PIC_BASE(0)+MPROC_STATUS_REG)>>28)+1;
}
```

### Initialization

Setup and SMP configuration

### Functions

1. GET_NRCPUS

	Usually used as the terminat condition of for-loop.

2. SET_NRCPUS

	This function is mentioned above


******
## contextTab

### Declaration

	//file core/kernel/arch/head.S
    ENTRY(contextTab)
		.zero CTXTTABSIZE

### Description


### Initialization


### Functions

1. hypercall SparcWritePtdL1Sys

	This function is EMPTY

2. SetupPtdL1

	//file core/kernel/arch/vmmap.c

	First, CloneXMPtdL1; save _pgTables content to ptdL1

    Second, update current guest's mmuCtxt, which is the current page table's backup

    Save page table backup into contextTab

3. LoadPartitionPageTable & SetMmuCtxt

	//file core/include/arch/processor.h

	Restore backuped page table when resetting kthreads and partitions

4. ASM arch/head.S

	load contextTab's physical address to a pointer.

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description


### Initialization


### Functions

1. GET_NRCPUS

2. SET_NRCPUS

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description


### Initialization


### Functions

1. GET_NRCPUS

2. SET_NRCPUS

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description


### Initialization


### Functions

1. GET_NRCPUS

2. SET_NRCPUS

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description


### Initialization


### Functions

1. GET_NRCPUS

2. SET_NRCPUS

******
## __nrCpus

### Declaration

	//file core/kernel/setup.c
    xm_u16_t __nrCpus = 0;

### Description


### Initialization


### Functions

1. GET_NRCPUS

2. SET_NRCPUS

