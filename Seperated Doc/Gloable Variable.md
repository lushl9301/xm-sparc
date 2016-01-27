#Global Varible

## xmcPartitionTab

### Declaration

	//file core/kernel/setup.c
	struct xmcPartition *xmcPartitionTab;

### Description

An array of ```xmcPartiton```. Length of this array is ```xmcTab.noPartitions```. ```xmcPartition``` struct consists of the id of partition, number of virtual CPUs assigned to this partition, communication ports of the partition, consoleDev, etc.. It is the detailed representation of XtratuM partition transalted from XML files.

One of the attribute ```xmcPartitionArch``` is empty.

### Initialization

Initialization is done using xmcparser and xml tools.


### Functions

1. SetupPartitions
xmcPartitionTab[e].noPhysicalMemoryAreas is used for print information about physical memory area.

2. CreatePartition
Assign xmcPartition to a partition_t p, an element in partitionTab.

For each virtual CPU, allocate one thread to partition, with flags cleared and timers  allocated.

******
## xmcMemRegTab

### Declaration

	//file core/kernel/setup.c
	struct xmcMemoryRegion *xmcMemRegTab;

### Description

An array of ```xmcMemoryRegion```. The length of this array is ```xmcTab.noRegions```, which is the number of regions of XtratuM. ```xmcMemoryRegion``` struct consists of the start address, size of the memory region and corresponding flags of it.

### Initialization

Initialized by parser and xml tools.

### Functions

1. PmmFindAnonymousPage
Binary search for a certain address.

2. PmmFindPage

3. PmmFindArea

4. PmmResetPartition 
	```
    page=&physPageTab[memArea->memoryRegionOffset][(addr-memRegion->startAddr)>>PAGE_SHIFT];
	```
    is used to find addr located page.
5. SetupPhysMM
Create empty physical memory of size:
```
//e for one of current xmcTab.noRegions
GET_MEMZ(physPageTab[e], sizeof(struct physPage)*(xmcMemRegTab[e].size/PAGE_SIZE))
```
and init spinlock for the area


******
## xmcPhysMemAreaTab

### Declaration

	//file core/kernel/setup.c
    struct xmcMemoryArea *xmcPhysMemAreaTab;

### Description
An array of ```xmcMemoryRegion```. The size of the array is the summ of ```xmcPartitonTab[0~xmcTab.noPartitions-1].noPhysicalMemoryAreas```.
Struct ```xmcMemoryArea``` consists of its starting address, mapped address, flags, as well as the size of this memory area.

This array is used mainly to record the memory size allocation. Array ```memBlockData``` is used to keep tracking the usage of memory of a ```kDevice_t```.

### Initialization

Initialized by parser and xml tools.

### Functions

1. ReadMemBlock
//file core/drivers/memblock.c

2. WriteMemBlock

3. SeekMemBlock

4. InitMemBlock
VmMapPage virtual memory paging operation here

5. SetupVmMap
//file core/kernel/arch/vmmap.c
Set flags and initial value of ```_ptdL3```

6. PmmFindPage & FindAddr & FindArea & ResetPartition
//file core/kernel/mmu/physmm.c
Uses xmcPhysMemAreaTab's memory region information.

7. SetupPageTable
//file core/kernel/mmu/vmmap.c
same as above

8. SetupPartitions
//file setup.c
same as above

******
## xmcCommChannelTab

### Declaration

	//file core/kernel/setup.c
	struct xmcCommChannel *xmcCommChannelTab;

### Description
An array of ```xmcCommChannel```  with size of ```xmcTab.noCommChannels```. Struct ```xmcCommChannel``` contains type and union of xm channel.

### Initialization

Initialized by parser and xml tools.

### Functions
It is used file core/objects/commports.c mainly (not considering ttnocports.c here).  Only provide configuration details.

******
## xmcCommPorts

### Declaration

	//file core/kernel/setup.c
	struct xmcCommChannel *xmcCommChannelTab;

### Description
Similar as above.

### Initialization

### Functions

******
## xmcIoPortTab

//TODO

### Declaration

	//file core/kernel/setup.c
    struct xmcIoPort *xmcIoPortTab;

### Description

### Initialization

### Functions

******
## xmcPartitionTab

### Declaration

	//file core/kernel/setup.c
	struct xmcMemoryRegion *xmcMemRegTab;

### Description


### Initialization

### Functions

1. SetupPartitions

2. CreatePartition


******
## xmcPartitionTab

### Declaration

	//file core/kernel/setup.c
	struct xmcMemoryRegion *xmcMemRegTab;

### Description


### Initialization

### Functions

1. SetupPartitions

2. CreatePartition


******
## xmcPartitionTab

### Declaration

	//file core/kernel/setup.c
	struct xmcMemoryRegion *xmcMemRegTab;

### Description


### Initialization

### Functions

1. SetupPartitions

2. CreatePartition
