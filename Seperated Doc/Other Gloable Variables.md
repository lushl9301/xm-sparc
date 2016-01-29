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

Save threads' contexts. contextTab is updated when saving / setting up L1 page table.

### Initialization

Filled up during _start ENTRY in core/kernel/arch/head.S. First fill up l3, l2, l1 page table and copy l1 content to the memory location pointed by contextTab.

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
## _pgTables[], _ptdL1[], _ptdL2[], _ptdL3[]

### Declaration

	//file core/kernel/arch/head.S
    .align 1024
    ENTRY(_pgTables)
    ENTRY(_ptdL1)
        .zero PTDL1SIZE = 1024
    /*???
    (16MB)
    -----------------
    4096 4KPAGES
    1 L1
    1 L2
    64 L3
    --------------
    */
    .align 256
    ENTRY(_ptdL2)
        .zero NO_PTDL2_XMVMAP*PTDL2SIZE = 1 * 256

    .align 256
    ENTRY(_ptdL3)
        .zero NO_PTDL3_XMVMAP*PTDL3SIZE = 64 * 256

### Description

SparcV8.pdf page 241. Level 1 size 1024, 256 enries. level 2 size 256, 64 entries. level 3 size 256, 64 entries. __pgTables starts at the same location as _ptdL1.

### Initialization

Filled up during _start ENTRY in core/kernel/arch/head.S.

### Functions

1. CloneXMPtdL1

	```
//write _pgTables -> ptdL1
WriteByPassMmuWord(&ptdL1[l1e], _pgTables[l1e]);
	```

2. _start

	//file core/kernel/arch/head.S

    Write ptdL3, store ptdL3 to ptdL2

    Write ptdL2, store into ptdL1 (CONFIG_XM_OFFSET and CONFIG_XM_LOAD_ADDR)

    Write ptdL1 to contextTab entry

3. SetupVmMap

	Put hypervisor's physical memory into ptdL3.

	And clean the frame.

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

