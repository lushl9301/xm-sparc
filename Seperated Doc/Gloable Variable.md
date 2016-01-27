#Global Varible

## xmcPartitionTab

### Declaration

	//file core/kernel/setup.c
	struct xmcPartition *xmcPartitionTab;

### Description

An array of ```xmcPartiton```. Length of this array is ```xmcTab.noPartitions```. ```xmcPartition``` struct consists of the id of partition, number of virtual CPUs assigned to this partition, communication ports of the partition, consoleDev, etc.. It is the detailed representation of XtratuM partition transalted from XML files.

One of the attribute of ```xmcPartition```, ```xmcPartitionArch```, is empty.

### Initialization

Initialization is done using xmcparser and xml tools.


### Functions

1. SetupPartitions
xmcPartitionTab[e].noPhysicalMemoryAreas is used for print information about physical memory area.

2. CreatePartition
Assign xmcPartition to a partition_t p, an element in partitionTab.
\#forloop
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

### Initialization

Initialized by parser and xml tools.

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
