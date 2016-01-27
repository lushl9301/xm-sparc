
#Structure in XtratuM

### List of XtratuM struct

```sh
egrep -rn "struct \w+ {" . > ../structlist
```

[List of XtratuM Structure](https://gist.github.com/lushl9301/320bf9bf731fe24a5073)

### ./core/drivers/leon_uart.c:85:static struct uartInfo

```c
static struct uartInfo {
    xm_u32_t base;
    xm_s32_t irq;
} uartInfo[]={
    [0]={
    .base=LEON_UART1_BASE,
        .irq=UART1_IRQ,
    },
    [1]={
    .base=LEON_UART2_BASE,
        .irq=UART2_IRQ,
    },
};
```

```c
#define LEON_UART1_BASE (LEON_IOREGS+0x100)
#define LEON_UART2_BASE (LEON_IOREGS+0x900)
```


### ./core/kernel/arch/leon_timers.c:142:static struct pitClockData

```c
static struct pitClockData {
    volatile xm_u32_t ticks;
} pitClockData={
    .ticks=0,
};
```

Usage Example:
```c
static void ClockIrqHandler(cpuCtxt_t *ctxt, void *irqData) {
    pitClockData.ticks++;
}
```

### ./core/kernel/irqs.c:82:extern struct exPTable

```c
extern struct exPTable {
    xmAddress_t a;
    xmAddress_t b;
} exPTable[];
```

```c
#from DoTrap()
if ((pc=IsInPartExTable(GET_CTXT_PC(ctxt)))) {
    SET_CTXT_PC(ctxt, pc);
    return;
}
```

Seems that exPTable.a == &ctxt; exPTable.b == pc

### ./core/klibc/stdio.c:283:typedef struct Sndata

```c
typedef struct Sndata {
    char *s;
    xm_s32_t *n;    // s size
    xm_s32_t *nc;
} Sndata;
```

This is used for print a string "s" with "n" length: snprintf.

### ./core/include/xmef.h:29:struct xefCustomFile

```c
struct xefCustomFile {
    xmAddress_t sAddr;
    xmSize_t size;
} __PACKED;

# struct xefCustomFile customFileTab[CONFIG_MAX_NO_CUSTOMFILES]; // define == 3
```

Only Address and Size is provided.

### ./core/include/xmef.h:67:struct xmefFile

```c
struct xmefFile {
    xmAddress_t offset;
    xmSize_t size;
    xmAddress_t nameOffset;
} __PACKED;
```

This code is from ldr.c

```struct xefFile xefFile,xefCustomFile```

which is really confusing

### ./core/include/xmef.h:39:struct xmHdr

```c
struct xmHdr {
#define XMEF_XM_MAGIC 0x24584d68 // $XMh
    xm_u32_t sSignature;
    xm_u32_t compilationXmAbiVersion; // XM's abi version
    xm_u32_t compilationXmApiVersion; // XM's api version
    xm_u32_t noCustomFiles;
    struct xefCustomFile customFileTab[CONFIG_MAX_NO_CUSTOMFILES];
    xm_u32_t eSignature;
} __PACKED;
```

This structure is used for carry ```customFileTab``` as well as ```noCustomFiles```.

### ./core/include/xmef.h:51:struct xmImageHdr


```
struct xmImageHdr {
#define XMEF_PARTITION_MAGIC 0x24584d69 // $XMi
    xm_u32_t sSignature;
    xm_u32_t compilationXmAbiVersion; // XM's abi version
    xm_u32_t compilationXmApiVersion; // XM's api version
/* pageTable is unused when MPU is set */
    xmAddress_t pageTable; // Physical address
/* pageTableSize is unused when MPU is set */
    xmSize_t pageTableSize;
    xm_u32_t noCustomFiles;
    struct xefCustomFile customFileTab[CONFIG_MAX_NO_CUSTOMFILES];
    xm_u32_t eSignature;
} __PACKED;
```

### ./core/include/xmef.h:75:struct xmefPartition

```c
struct xmefPartition {
    xm_s32_t id;
    xm_s32_t file;
    xm_u32_t noCustomFiles;
    xm_s32_t customFileTab[CONFIG_MAX_NO_CUSTOMFILES];
} __PACKED;
```

```
./user/tools/xmpack/xmpack.c:struct xmefPartition *partitionTab=0, *hypervisor;
```

"xmpack" is used for "Create a pack holding the image of XM and partitions to be written into the ROM"

### ./core/include/xmef.h:87:struct xmefContainerHdr

```c
struct xmefContainerHdr {
    xm_u32_t signature;
#define XM_PACKAGE_SIGNATURE 0x24584354 // $XCT
    xm_u32_t version;
#define XMPACK_VERSION 3
#define XMPACK_SUBVERSION 0
#define XMPACK_REVISION 0
    xm_u32_t flags;
#define XMEF_CONTAINER_DIGEST 0x1
    xm_u8_t digest[XM_DIGEST_BYTES];
    xm_u32_t fileSize;
    xmAddress_t partitionTabOffset;
    xm_s32_t noPartitions;
    xmAddress_t fileTabOffset;
    xm_s32_t noFiles;
    xmAddress_t strTabOffset;
    xm_s32_t strLen;
    xmAddress_t fileDataOffset;
    xmSize_t fileDataLen;    
} __PACKED;
```

This struct contains offset; This struct is included in ```xefContainerFile```; This struct can be used as img, which read data from.

### ./core/include/xmef.h:114:struct xefHdr

```c
struct xefHdr {
#define XEF_SIGNATURE 0x24584546
    xm_u32_t signature;
    xm_u32_t version;
#define XEF_DIGEST 0x1
#define XEF_COMPRESSED 0x4
#define XEF_RELOCATABLE 0x10

#define XEF_TYPE_MASK 0xc0
#define XEF_TYPE_HYPERVISOR 0x00
#define XEF_TYPE_PARTITION 0x40
#define XEF_TYPE_CUSTOMFILE 0x80

#define XEF_ARCH_SPARCv8 0x400
#define XEF_ARCH_MASK 0xff00
    xm_u32_t flags;
    xm_u8_t digest[XM_DIGEST_BYTES];
    xm_u8_t payload[XM_PAYLOAD_BYTES];
    xmSize_t fileSize;
    xmAddress_t segmentTabOffset;
    xm_s32_t noSegments;
    xmAddress_t customFileTabOffset;
    xm_s32_t noCustomFiles;
    xmAddress_t imageOffset;
    xmSize_t imageLength;
    xmSize_t deflatedImageLength;
    xmAddress_t pageTable;
    xmSize_t pageTableSize;
    xmAddress_t xmImageHdr;
    xmAddress_t entryPoint;
} __PACKED;
```


### ./core/include/xmef.h:147:struct xefSegment

```c
struct xefSegment {
    xmAddress_t physAddr;
    xmAddress_t virtAddr;
    xmSize_t fileSize;
    xmSize_t deflatedFileSize;
    xmAddress_t offset;
} __PACKED;
```

//TODO

### ./core/include/drivers/memblock.h:19:struct memBlockData
```c
struct memBlockData {
    xm_s32_t pos;
    xmAddress_t addr;
    struct xmcMemBlock *cfg;
};
```

```c
struct xmcMemBlock {
    xm_u32_t physicalMemoryAreasOffset;
};
```

### ./core/include/objects/console.h:22:struct console
```c
struct console {
    const kDevice_t *dev;
};
```

For example
```c
static struct console xmCon, *partitionConTab;
void ConsolePutChar(xm_u8_t c) {
    if (xmCon.dev) {
        if (KDevWrite(xmCon.dev, &c, 1)!=1) {
            KDevSeek(xmCon.dev, 0, DEV_SEEK_START);
            KDevWrite(xmCon.dev, &c, 1);
        }
    }
}
```

Console is a mounted device.

### ./core/include/objects/trace.h:25:struct xmTraceEvent



### ./core/include/objects/commports.h:95:struct createSCmd
```c
union samplingPortCmd {
    struct createSCmd {
        char *__gParam portName;
        xm_u32_t maxMsgSize;
        xm_u32_t direction;
        xmTime_t validPeriod;
    } create;
    xmSamplingPortStatus_t status;
    xmSamplingPortInfo_t info;
};
```
### ./core/include/objects/commports.h:107:struct createTCmd
```c
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
union ttnocPortCmd {
    struct createTCmd {
        char *__gParam portName;
        xm_u32_t maxMsgSize;
        xm_u32_t direction;
        xmTime_t validPeriod;
    } create;
    xmTTnocPortStatus_t status;
    xmTTnocPortInfo_t info;
};
#endif
```
### ./core/include/objects/commports.h:119:struct createQCmd
```c
union queuingPortCmd {
    struct createQCmd {
        char *__gParam portName;
        xm_u32_t maxNoMsgs;
        xm_u32_t maxMsgSize;
        xm_u32_t direction;
    } create;
    xmQueuingPortStatus_t status;
    xmQueuingPortInfo_t info;
};
```

- ```SCmd``` and ```TCmd``` are the same.

- ```QCmd``` has extra variable named ```maxNoMsgs```.

### ./core/include/objects/commports.h:161:union channel

The struct ```msg``` is contained in ```union channel```'s ```q``` struct as a linked-list.

```c
union channel {
    struct {
        char *buffer;
        xm_s32_t length;
        xmTime_t timestamp;
        partition_t **receiverTab;
        xm_s32_t *receiverPortTab;
        xm_s32_t noReceivers;
        partition_t *sender;
        xm_s32_t senderPort;
        spinLock_t lock;
    } s;
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
    struct {
//          Don't need buffer?
//          char *buffer;
        xm_s32_t length;
        xmTime_t timestamp;
        partition_t **receiverTab;
        xm_s32_t *nodeId;
        xm_s32_t *receiverPortTab;
        xm_s32_t noReceivers;
        partition_t *sender;
        xm_s32_t senderPort;
        spinLock_t lock;
    } t;
#endif
    struct {
        struct msg {
            struct dynListNode listNode;
            char *buffer;
            xm_s32_t length;
            xmTime_t timestamp;
        } *msgPool;
        struct dynList freeMsgs, recvMsgs;
        xm_s32_t usedMsgs;
        partition_t *receiver;
        xm_s32_t receiverPort;
        partition_t *sender;
        xm_s32_t senderPort;
        spinLock_t lock;
    } q;
};
```

### ./core/include/objects/commports.h:177:struct port

```c
struct port {   
    xm_u32_t flags;
#define COMM_PORT_OPENED 0x1
#define COMM_PORT_EMPTY 0x0
#define COMM_PORT_NEW_MSG 0x2
#define COMM_PORT_CONSUMED_MSG 0x4
#define COMM_PORT_MSG_MASK 0x6
    xmId_t partitionId;
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
    kDevice_t *ttnocDev;
#endif
    spinLock_t lock;
};
```

```*portTab``` is used to keep records of all communication ports.

### ./core/include/objects/mem.h:19:struct cpyArea
```c
union memCmd {
    struct cpyArea {
        xmId_t dstId;
        xmAddress_t dstAddr;
        xmId_t srcId;
        xmAddress_t srcAddr;
        xmSSize_t size;
    } cpyArea;
};
```

This ```union``` is used to eliminate multiple parameters passing problem.

For example:
```c
return CopyArea(args->cpyArea.dstAddr, args->cpyArea.dstId,
    args->cpyArea.srcAddr, args->cpyArea.srcId, args->cpyArea.size);
```

### ./core/include/objects/hm.h:18:struct hmCpuCtxt

```c
struct hmCpuCtxt {
    xm_u32_t pc;
    xm_u32_t nPc;
    xm_u32_t psr;
};
```

This is different from ```ctxt``` which is of ```struct cputCtxt_t```. ```ctxt``` contains current register status.

This structure contains less information, and is mainly involved from ```struct xmHmLog```.

```c
#define CpuCtxt2HmCpuCtxt(cpuCtxt, hmCpuCtxt) do { \
    (hmCpuCtxt)->nPc=(cpuCtxt)->nPc; \
    (hmCpuCtxt)->psr=(cpuCtxt)->psr; \
    (hmCpuCtxt)->pc=(cpuCtxt)->pc; \
} while(0)
```


### ./core/include/objects/hm.h:25:struct xmHmLog
```c
struct xmHmLog {
#define XM_HMLOG_SIGNATURE 0xfecf
    xm_u16_t signature;
    xm_u16_t checksum;
    xm_u32_t opCodeH, opCodeL;
// HIGH
#define HMLOG_OPCODE_SEQ_MASK (0xfffffff0<<HMLOG_OPCODE_SEQ_BIT)
#define HMLOG_OPCODE_SEQ_BIT 4
#define HMLOG_OPCODE_VALID_CPUCTXT_MASK (0x1<<HMLOG_OPCODE_VALID_CPUCTXT_BIT)
#define HMLOG_OPCODE_VALID_CPUCTXT_BIT 1
#define HMLOG_OPCODE_SYS_MASK (0x1<<HMLOG_OPCODE_SYS_BIT)
#define HMLOG_OPCODE_SYS_BIT 0
// LOW
#define HMLOG_OPCODE_EVENT_MASK (0xffff<<HMLOG_OPCODE_EVENT_BIT)
#define HMLOG_OPCODE_EVENT_BIT 16
// 256 vcpus
#define HMLOG_OPCODE_VCPUID_MASK (0xff<<HMLOG_OPCODE_VCPUID_BIT)
#define HMLOG_OPCODE_VCPUID_BIT 8
// 256 partitions
#define HMLOG_OPCODE_PARTID_MASK (0xff<<HMLOG_OPCODE_PARTID_BIT)
#define HMLOG_OPCODE_PARTID_BIT 0
    xmTime_t timestamp;
    union {
#define XM_HMLOG_PAYLOAD_LENGTH 4
        struct hmCpuCtxt cpuCtxt;
        xmWord_t payload[XM_HMLOG_PAYLOAD_LENGTH];
    };
}  __PACKED;
```

```xmHmLog``` is just ```xmHmLog_t```
Most likely, ```opCodeH``` and ```opCodeL``` are used. Other attributes are not used but some of them are assigned.

### ./core/include/digest.h:19:struct digestCtx
```c
struct digestCtx {
    xm_u8_t in[64];
    xm_u32_t buf[4];
    xm_u32_t bits[2];
};
```
Structure for MD5 (Message Digest) computation.

### ./core/include/list.h:24:struct dynListNode
### ./core/include/list.h:29:struct dynList

```c
struct dynList;
struct dynListNode {
    struct dynList *list;
    struct dynListNode *prev, *next;
};
```
```c
struct dynList {
    struct dynListNode *head;
    xm_s32_t noElem;
    spinLock_t lock;
};
```
Simple data structure, easy to understand.

### ./core/include/arch/irqs.h:23:struct trapHandler
```c
struct trapHandler {
    xmAddress_t pc;
};
```
Just use Program Counter to redirect trapHandler.

### ./core/include/arch/irqs.h:32:typedef struct _cpuCtxt
```c
typedef struct _cpuCtxt {
    struct _cpuCtxt *prev; // a linked list?
    xmWord_t unused;
    xmWord_t y;
    xmWord_t g1;
    xmWord_t g2;
    xmWord_t g3;
    xmWord_t g4;
    xmWord_t g5;
    xmWord_t g6;
    xmWord_t g7;
    xmWord_t nPc;
    xmWord_t irqNr;
    xmWord_t psr;
    xmWord_t pc;
} cpuCtxt_t;
```
A super-class of ```hmCpuCtxt```. This struct contains registers' information.

### ./core/include/arch/xmconf.h:66:struct xmcIoPort
```c
struct xmcIoPort {
    xm_u32_t type;
#define XM_IOPORT_RANGE 0
#define XM_RESTRICTED_IOPORT 1
    union {
        struct xmcIoPortRange {
            xmIoAddress_t base;
            xm_s32_t noPorts;
        } range;
        struct xmcRestrictdIoPort {
            xmIoAddress_t address;
            xm_u32_t mask;
#define XM_DEFAULT_RESTRICTED_IOPORT_MASK (~0)
        } restricted;
    };
};
```

### ./core/include/arch/guest.h:23:struct pctArch
```c
struct pctArch {
    xmAddress_t tbr;
#ifdef CONFIG_MMU
    volatile xmAddress_t ptdL1;
#define _ARCH_PTDL1_REG ptdL1
    volatile xm_u32_t faultStatusReg;
    volatile xm_u32_t faultAddressReg;
#endif
};
```

This structure is visible from the guest. It is included in ```partitionControlTable_t``` as Arch.

```c
// core/kernel/mmu/kthread.c
void SetupPctMm(partitionControlTable_t *partCtrlTab, kThread_t *k) {
    ///???
    // if defined MPU, ptdL1 will not be declared in kThreadArch struct
    // check include/arch/kthread.h
    partCtrlTab->arch._ARCH_PTDL1_REG=k->ctrl.g->kArch.ptdL1;
}
```

### ./core/include/arch/kthread.h:25:struct kThreadArch
```
struct kThreadArch {
#if defined(CONFIG_MPU)
    xm_u32_t wpReg;
#else
    xmAddress_t ptdL1;
    xmAddress_t xPtdL1;
    xm_u8_t mmuCtxt;
#endif
    // TODO
    xm_u32_t tbr;
    // Floating point registers
    xm_u32_t fpuRegs[XM_FPU_REG_NR] ALIGNED_C;
};
```

This struct is usually contained in ```guest``` structure. Usually accessed by ```ctrl.g->kArch```.

### ./core/include/irqs.h:34:struct irqTabEntry
```c
struct irqTabEntry {
    irqHandler_t handler;
    void *data;
};
```

This structure is used for irq array: ```struct irqTabEntry irqHandlerTab[CONFIG_NO_HWIRQS];```

Usage:
```c
if (irqHandlerTab[ctxt->irqNr].handler)
    (*(irqHandlerTab[ctxt->irqNr].handler))(ctxt, irqHandlerTab[ctxt->irqNr].data);
else
    DefaultIrqHandler(ctxt, 0);
```

### ./core/include/xmconf.h:27:struct xmcHmSlot
```c
struct xmcHmSlot {
    xm_u32_t action:31, log:1;
// Logging
#define XM_HM_LOG_DISABLED 0
#define XM_HM_LOG_ENABLED 1
// Actions
#define XM_HM_AC_IGNORE 0
#define XM_HM_AC_SHUTDOWN 1
#define XM_HM_AC_PARTITION_COLD_RESET 2
#define XM_HM_AC_PARTITION_WARM_RESET 3
#define XM_HM_AC_HYPERVISOR_COLD_RESET 4
#define XM_HM_AC_HYPERVISOR_WARM_RESET 5
#define XM_HM_AC_SUSPEND 6
#define XM_HM_AC_PARTITION_HALT 7
#define XM_HM_AC_HYPERVISOR_HALT 8
#define XM_HM_AC_PROPAGATE 9
#define XM_HM_AC_SWITCH_TO_MAINTENANCE 10
#define XM_HM_MAX_ACTIONS 11
};
```
### ./core/include/xmconf.h:78:struct xmcCommPort
```c
struct xmcCommPort {
    xm_u32_t nameOffset;
    xm_s32_t channelId;
#define XM_NULL_CHANNEL -1
    xm_s32_t direction;
#define XM_SOURCE_PORT 0x2
#define XM_DESTINATION_PORT 0x1
    xm_s32_t type;
#define XM_SAMPLING_PORT 0
#define XM_QUEUING_PORT 1
#define XM_TTNOC_PORT 2
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
    xmDev_t devId;
#endif
};
```
### ./core/include/xmconf.h:96:struct xmcSchedCyclicSlot
```c
struct xmcSchedCyclicSlot {
    xmId_t id;
    xmId_t partitionId;
    xmId_t vCpuId;
    xm_u32_t sExec; // offset (usec)
    xm_u32_t eExec; // offset+duration (usec)
};
```
### ./core/include/xmconf.h:105:struct xmcSchedCyclicPlan
```c
struct xmcSchedCyclicPlan {
    xm_u32_t nameOffset;
    xmId_t id;
    xm_u32_t majorFrame; // in useconds
    xm_s32_t noSlots;
#ifdef CONFIG_PLAN_EXTSYNC
    xm_s32_t extSync; // -1 means no sync
#endif
    xm_u32_t slotsOffset;
};
```
### ./core/include/xmconf.h:118:struct xmcMemoryArea
```c
struct xmcMemoryArea {
    xm_u32_t nameOffset;
    xmAddress_t startAddr;
    xmAddress_t mappedAt;
    xmSize_t size;
#define XM_MEM_AREA_SHARED (1<<0)
#define XM_MEM_AREA_UNMAPPED (1<<1)
#define XM_MEM_AREA_READONLY (1<<2)
#define XM_MEM_AREA_UNCACHEABLE (1<<3)
#define XM_MEM_AREA_ROM (1<<4)
#define XM_MEM_AREA_FLAG0 (1<<5)
#define XM_MEM_AREA_FLAG1 (1<<6)
#define XM_MEM_AREA_FLAG2 (1<<7)
#define XM_MEM_AREA_FLAG3 (1<<8)
#define XM_MEM_AREA_TAGGED (1<<9)
#define XM_MEM_AREA_IOMMU (1<<10)
    xm_u32_t flags;
    xm_u32_t memoryRegionOffset;
};
```
### ./core/include/xmconf.h:139:struct xmcRsw
```c
struct xmcRsw {
    xm_s32_t noPhysicalMemoryAreas;
    xm_u32_t physicalMemoryAreasOffset;
    //xmAddress_t entryPoint;
};
```
### ./core/include/xmconf.h:145:struct xmcTrace
```c
struct xmcTrace {
    xmDev_t dev;
    xm_u32_t bitmap;
};
```
### ./core/include/xmconf.h:150:struct xmcPartition
```c
struct xmcPartition {
    xmId_t id;
    xm_u32_t nameOffset;
    xm_u32_t flags;
#define XM_PART_SYSTEM 0x100
#define XM_PART_FP 0x200
    xm_u32_t noVCpus;
    xm_u32_t hwIrqs;
    xm_s32_t noPhysicalMemoryAreas;
    xm_u32_t physicalMemoryAreasOffset;
    xmDev_t consoleDev;    
    struct xmcPartitionArch arch;
    xm_u32_t commPortsOffset;
    xm_s32_t noPorts;
    struct xmcHmSlot hmTab[XM_HM_MAX_EVENTS];
    xm_u32_t ioPortsOffset;
    xm_s32_t noIoPorts;
    struct xmcTrace trace;
    struct xmcPartIpvi {
        xm_u32_t dstOffset;
        xm_s32_t noDsts;
    } ipviTab[CONFIG_XM_MAX_IPVI];
};
```
### ./core/include/xmconf.h:175:struct xmcCommChannel
```c
struct xmcCommChannel {
#define XM_SAMPLING_CHANNEL 0
#define XM_QUEUING_CHANNEL 1
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
#define XM_TTNOC_CHANNEL 2
#endif
    xm_s32_t type;

    union {
        struct {
            xm_s32_t maxLength;
            xm_s32_t maxNoMsgs;
        } q;
        struct {
            xm_s32_t maxLength;
            xm_u32_t validPeriod;
            xm_s32_t noReceivers;
        } s;
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
        struct {
            xm_s32_t maxLength;
            xm_u32_t validPeriod;
            xm_s32_t noReceivers;
            xmId_t nodeId;
        } t;
#endif
    };
};
```
### ./core/include/xmconf.h:205:struct xmcMemoryRegion
```c
struct xmcMemoryRegion {
    xmAddress_t startAddr;
    xmSize_t size;
#define XMC_REG_FLAG_PGTAB (1<<0)
#define XMC_REG_FLAG_ROM (1<<1)
    xm_u32_t flags;
};
```
### ./core/include/xmconf.h:213:struct xmcHwIrq
```c
struct xmcHwIrq {
    xm_s32_t owner;
#define XM_IRQ_NO_OWNER -1
};
```
### ./core/include/xmconf.h:218:struct xmcHpv
```
struct xmcHpv {
    xm_s32_t noPhysicalMemoryAreas;
    xm_u32_t physicalMemoryAreasOffset;
    xm_s32_t noCpus;
    struct _cpu {
        xmId_t id;
        xm_u32_t features; // Enable/disable features
        xm_u32_t freq; // KHz
#define XM_CPUFREQ_AUTO 0

#define CYCLIC_SCHED 0
#define FP_SCHED 1
        xm_u32_t schedPolicy;
#ifdef CONFIG_CYCLIC_SCHED
        xm_u32_t schedCyclicPlansOffset;
        xm_s32_t noSchedCyclicPlans;
#endif
#ifdef CONFIG_FP_SCHED
        xm_u32_t schedFpTabOffset;
        xm_s32_t noFpEntries;
#endif
    } cpuTab[CONFIG_NO_CPUS];
    struct xmcHpvArch arch;
    struct xmcHmSlot hmTab[XM_HM_MAX_EVENTS];
    xmDev_t hmDev;
    xmDev_t consoleDev;
    xmId_t nodeId;
    struct xmcHwIrq hwIrqTab[CONFIG_NO_HWIRQS];  
    struct xmcTrace trace;
};
```
### ./core/include/xmconf.h:250:struct xmcMemBlock
```c
struct xmcMemBlock {
    xm_u32_t physicalMemoryAreasOffset;
};
```
### ./core/include/xmconf.h:256:struct xmcTTnocSlot
```c
struct xmcTTnocSlot {
    xmAddress_t ttsocId;
    xmSize_t size;
    xm_s32_t type;
    xmId_t nodeId;
    xmDev_t devId;
};
```
### ./core/include/xmconf.h:264:struct xmcDevice
```
struct xmcDevice {
#if defined(CONFIG_DEV_MEMBLOCK)||defined(CONFIG_DEV_MEMBLOCK_MODULE)
    xmAddress_t memBlocksOffset;
    xm_s32_t noMemBlocks;
#endif
#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
    xmAddress_t ttnocSlotOffset;
    xm_s32_t noTTnocSlots;
#endif
#if defined(CONFIG_DEV_UART)||defined(CONFIG_DEV_UART_MODULE)
    struct xmcUartCfg {
        xm_u32_t baudRate;
    } uart[CONFIG_DEV_NO_UARTS];
#endif
#ifdef CONFIG_DEV_VGA
    struct xmcVgaCfg {

    } vga;
#endif
};
```
### ./core/include/xmconf.h:285:struct xmcRsvMem
```
struct xmcRsvMem {
    void *obj;
    xm_u32_t usedAlign;
#define RSV_MEM_USED 0x80000000
    xm_u32_t size;
} __PACKED;
```
### ./core/include/xmconf.h:292:struct xmcBootPart
```c
struct xmcBootPart {
#define XM_PART_BOOT 0x1
    xm_u32_t flags;
    xmAddress_t hdrPhysAddr;
    xmAddress_t entryPoint;
    xmAddress_t imgStart;
    xmSize_t imgSize;
    xm_u32_t noCustomFiles;
    struct xefCustomFile customFileTab[CONFIG_MAX_NO_CUSTOMFILES];
};
```
### ./core/include/xmconf.h:303:struct xmcRswInfo
```c
struct xmcRswInfo {
    xmAddress_t entryPoint;
};
```
### ./core/include/xmconf.h:308:struct xmcFpSched
```c
struct xmcFpSched {
    xmId_t partitionId;
    xmId_t vCpuId;
    xm_u32_t priority;
};
```
### ./core/include/xmconf.h:323:struct xmc
This structure contains every important part of the XtratuM.
```
struct xmc {
#define XMC_SIGNATURE 0x24584d43 // $XMC
    xm_u32_t signature;
    xm_u8_t digest[XM_DIGEST_BYTES];
    xmSize_t dataSize;
    xmSize_t size;
// Reserved(8).VERSION(8).SUBVERSION(8).REVISION(8)
#define XMC_SET_VERSION(_ver, _subver, _rev) ((((_ver)&0xFF)<<16)|(((_subver)&0xFF)<<8)|((_rev)&0xFF))
#define XMC_GET_VERSION(_v) (((_v)>>16)&0xFF)
#define XMC_GET_SUBVERSION(_v) (((_v)>>8)&0xFF)
#define XMC_GET_REVISION(_v) ((_v)&0xFF)
    xm_u32_t version;
    xm_u32_t fileVersion;
    xmAddress_t rsvMemTabOffset;
    xmAddress_t nameOffset;
    struct xmcHpv hpv;
    struct xmcRsw rsw;
    xmAddress_t partitionTabOffset;    
    xm_s32_t noPartitions;
    xmAddress_t bootPartitionTabOffset;
    xmAddress_t rswInfoOffset;
    xmAddress_t memoryRegionsOffset;
    xm_u32_t noRegions;
#ifdef CONFIG_CYCLIC_SCHED
    xmAddress_t schedCyclicSlotsOffset;
    xm_s32_t noSchedCyclicSlots;
    xmAddress_t schedCyclicPlansOffset;
    xm_s32_t noSchedCyclicPlans;
#endif
    xmAddress_t commChannelTabOffset;
    xm_s32_t noCommChannels;
    xmAddress_t physicalMemoryAreasOffset;
    xm_s32_t noPhysicalMemoryAreas;
    xmAddress_t commPortsOffset;
    xm_s32_t noCommPorts;
    xmAddress_t ioPortsOffset;
    xm_s32_t noIoPorts;
    xmAddress_t ipviDstOffset;
    xm_s32_t noIpviDsts;
#ifdef CONFIG_FP_SCHED
    xmAddress_t fpSchedTabOffset;
    xm_s32_t noFpEntries;
#endif
    xmAddress_t vCpuTabOffset;
    xmAddress_t stringsOffset;
    xm_s32_t stringTabLength;
    struct xmcDevice deviceTab;
} __PACKED;
```

### ./core/include/physmm.h:30:struct physPage
```c
struct physPage {
    struct dynListNode listNode;
#ifndef CONFIG_ARCH_MMU_BYPASS
    xmAddress_t vAddr;
#endif
    xm_u32_t mapped:1, unlocked:1, type:3, counter:27;
    spinLock_t lock;
};
```

```c
./core/kernel/mmu/physmm.c:static struct physPage **physPageTab;
./core/kernel/arch/hypercalls.c:    struct physPage *ptdL1Page;
./core/kernel/arch/vmmap.c:    struct physPage *pagePtdL2, *pagePtdL3;
```

###/include/guest.h:79:struct xmPhysicalMemMap
```c
struct xmPhysicalMemMap {
    xm_s8_t name[CONFIG_ID_STRING_LENGTH];
    xmAddress_t startAddr;
    xmAddress_t mappedAt;
    xmSize_t size;
#define XM_MEM_AREA_SHARED          (1<<0)
#define XM_MEM_AREA_UNMAPPED        (1<<1)
#define XM_MEM_AREA_READONLY        (1<<2)
#define XM_MEM_AREA_UNCACHEABLE     (1<<3)
#define XM_MEM_AREA_ROM             (1<<4)
#define XM_MEM_AREA_FLAG0           (1<<5)
#define XM_MEM_AREA_FLAG1           (1<<6)
#define XM_MEM_AREA_FLAG2           (1<<7)
#define XM_MEM_AREA_FLAG3           (1<<8)
#define XM_MEM_AREA_TAGGED          (1<<9)
    xm_u32_t flags;
};
```
Use ```memMap[e].name``` to distinguish different memory area.

### ./core/include/guest.h:100:struct schedInfo
```c
struct schedInfo {
    xm_u32_t noSlot;
    xm_u32_t id;
    xm_u32_t slotDuration;
};
```
### ./core/include/guest.h:108:struct partitionControlTable_t
```c
typedef struct {
    xm_u32_t magic;
    xm_u32_t xmVersion; // XM version
    xm_u32_t xmAbiVersion; // XM's abi version
    xm_u32_t xmApiVersion; // XM's api version
    xmSize_t partCtrlTabSize;
    xm_u32_t resetCounter;
    xm_u32_t resetStatus;
    xm_u32_t cpuKhz;
#define PCT_GET_PARTITION_ID(pct) ((pct)->id&0xff)
#define PCT_GET_VCPU_ID(pct) ((pct)->id>>8)
    xmId_t id;
    xmId_t noVCpus;
    xmId_t schedPolicy;
    xm_u32_t flags;
    xm_u32_t imgStart;
    xm_u32_t hwIrqs; // Hw interrupts belonging to the partition
    xm_s32_t noPhysicalMemAreas;
    xm_s32_t noCommPorts;
    xm_u8_t name[CONFIG_ID_STRING_LENGTH]; //seems to be 16?
    xm_u32_t iFlags; // As defined by the ARCH (ET+PIL in sparc)
    xm_u32_t hwIrqsPend; // pending hw irqs //cannot nested IRQ?
    xm_u32_t hwIrqsMask; // masked hw irqs

    xm_u32_t extIrqsPend; // pending extended irqs
    xm_u32_t extIrqsMask; // masked extended irqs

    struct pctArch arch;
    struct schedInfo schedInfo;
    xm_u16_t trap2Vector[NO_TRAPS];
    xm_u16_t hwIrq2Vector[CONFIG_NO_HWIRQS];
    xm_u16_t extIrq2Vector[XM_VT_EXT_MAX];
} partitionControlTable_t;
```

### ./core/include/smp.h:27:struct localId
```c
// This structure is stored by each processor
struct localId {
    xm_u32_t id; // logical ID
    xm_u32_t hwId; // HW ID
} __PACKED;

// struct localId localIdTab[CONFIG_NO_CPUS];
// but this array is never used
```

### ./core/include/kdevice.h:26:typedef struct kDev
```c
typedef struct kDev {
    xm_u16_t subId;
    xm_s32_t (*Reset)(const struct kDev *);
    xm_s32_t (*Write)(const struct kDev *, xm_u8_t *buffer, xm_s32_t len);
    xm_s32_t (*Read)(const struct kDev *, xm_u8_t *buffer, xm_s32_t len);
    xm_s32_t (*Seek)(const struct kDev *, xm_u32_t offset, xm_u32_t whence);
} kDevice_t;
```
This struct is similiar as Linux Device interface
In XtratuM:
All I/O device are device; memBlock are devices so that read, write, reset, seek can be mounted to device interface
```
./core/include/logstream.h:    const kDevice_t *kDev;
./core/include/drivers/ttnocports.h:    kDevice_t *txSlot;
./core/include/drivers/ttnocports.h:    kDevice_t *rxSlot;
./core/include/objects/console.h:    const kDevice_t *dev;
./core/include/objects/commports.h:    kDevice_t *ttnocDev;
./core/drivers/memblock.c:static kDevice_t *memBlockTab=0;
./core/drivers/ttnocports.c:static kDevice_t *ttnocPortTab=0;
./core/drivers/pc_vga.c:static const kDevice_t textVga;
./core/drivers/pc_uart.c:static const kDevice_t uartDev={
./core/drivers/leon_uart.c:static kDevice_t uartTab[CONFIG_DEV_NO_UARTS];
```

### ./core/include/kthread.h:37:struct guest
```c
struct guest {
#define PART_VCPU_ID2KID(partId, vCpuId) ((vCpuId)<<8)|((partId)&0xff)
#define KID2PARTID(id) ((id)&0xff)
#define KID2VCPUID(id) ((id)>>8)
    xmId_t id; // 15..8: vCpuId, 7..0: partitionId
    struct kThreadArch kArch;
    vTimer_t vTimer;
    kTimer_t kTimer;
    kTimer_t watchdogTimer;
    vClock_t vClock;
    xm_u32_t opMode; /*Only for debug vcpus*/
    partitionControlTable_t *partCtrlTab;
    xm_u32_t swTrap;
    struct trapHandler overrideTrapTab[NO_TRAPS];
};
```

### ./core/include/kthread.h:69:struct __kThread
```c
typedef union kThread {
    struct __kThread {
        // Harcoded, don't change it
        xm_u32_t magic1;
        // Harcoded, don't change it
        xmAddress_t *kStack;
        spinLock_t lock;
        volatile xm_u32_t flags;
//  [3...0] -> scheduling bits
#define KTHREAD_FP_F (1<<1) // Floating point enabled
#define KTHREAD_HALTED_F (1<<2)  // 1:HALTED
#define KTHREAD_SUSPENDED_F (1<<3) // 1:SUSPENDED
#define KTHREAD_READY_F (1<<4) // 1:READY
#define KTHREAD_FLUSH_CACHE_B 5
#define KTHREAD_FLUSH_CACHE_W 3
#define KTHREAD_FLUSH_DCACHE_F (1<<5)
#define KTHREAD_FLUSH_ICACHE_F (1<<6)
#define KTHREAD_CACHE_ENABLED_B 7
#define KTHREAD_CACHE_ENABLED_W 3
#define KTHREAD_DCACHE_ENABLED_F (1<<7)
#define KTHREAD_ICACHE_ENABLED_F (1<<8)

#define KTHREAD_NO_PARTITIONS_FIELD (0xff<<16) // No. partitions
#define KTHREAD_TRAP_PENDING_F (1<<31) // 31: PENDING

        struct dynList localActiveKTimers;
        struct guest *g;
        void *schedData;
        cpuCtxt_t *irqCpuCtxt;
        xm_u32_t irqMask;
        xm_u32_t magic2;
    } ctrl;
    xm_u8_t kStack[CONFIG_KSTACK_SIZE];
} kThread_t;
```

It is common to see: ```kThread->ctrl.g```; ```ctrl``` is ```__kThread```; ```g``` is the ```guest```.

### ./core/include/kthread.h:128:typedef struct partition
```c
typedef struct partition {
    kThread_t **kThread;
    xmAddress_t pctArray;
    xmSize_t pctArraySize;
    xm_u32_t opMode;
    xmAddress_t imgStart; /*Partition Memory address in the container*/
    ///??? container?
    xmAddress_t vLdrStack; /*Stack address allocated by XM*/
    struct xmcPartition *cfg;
} partition_t;
```

//TODO

### ./core/include/sched.h:33:struct schedData
### ./core/include/sched.h:35:struct cyclicData
### ./core/include/sched.h:53:struct fpData
```
struct schedData {
#ifdef CONFIG_CYCLIC_SCHED
    struct cyclicData {
        kTimer_t kTimer;
        struct {
            const struct xmcSchedCyclicPlan *current;
            const struct xmcSchedCyclicPlan *new;
            const struct xmcSchedCyclicPlan *prev;
        } plan;
        xm_s32_t slot; // next slot to be processed
        xmTime_t mjf;
        xmTime_t sExec;
        xmTime_t planSwitchTime;
        xmTime_t nextAct;
        kThread_t *kThread;
        xm_u32_t flags;
#define RESCHED_ENABLED 0x1
    } cyclic;
#endif
#ifdef CONFIG_FP_SCHED
    struct fpData {
        struct xmcFpSched *fpTab;
        xm_s32_t noFpEntries;
    } fp;
#endif
};
```

```c
typedef struct {
    kThread_t *idleKThread;
    kThread_t *cKThread;
    kThread_t *fpuOwner; // not really used
    struct schedData *data;
    xm_u32_t flags;
#define LOCAL_SCHED_ENABLED 0x1
    kThread_t *(*GetReadyKThread)(struct schedData *schedData);
} localSched_t;
```

Most important part is the ```plan```.

### ./core/include/ktimer.h:32:typedef struct hwClock
```c
typedef struct hwClock {
    char *name;
    xm_u32_t flags;
#define HWCLOCK_ENABLED (1<<0)
#define PER_CPU (1<<1)
    xm_u32_t freqKhz;
    xm_s32_t (*InitClock)(void);
    xmTime_t (*GetTimeUsec)(void);
    void (*ShutdownClock)(void);
} hwClock_t;
```

Basic clock device. There are two clock types in Xtratum.
One is ```HwClock_t```, the other one is ```vClock_t```.

### ./core/include/ktimer.h:46:typedef struct hwTimer
```c
typedef struct hwTimer {
    xm_s8_t *name;
    xm_u32_t flags;
#define HWTIMER_ENABLED (1<<0)
    xm_u32_t freqKhz;    
    xm_s32_t irq;
    xm_s32_t (*InitHwTimer)(void);
    void (*SetHwTimer)(xmTime_t);
    // This is the maximum value to be programmed
    xmTime_t (*GetMaxInterval)(void);
    xmTime_t (*GetMinInterval)(void);
    timerHandler_t (*SetTimerHandler)(timerHandler_t);
    void (*ShutdownHwTimer)(void);
} hwTimer_t;
```

### ./core/include/ktimer.h:63:typedef struct kTimer

```c
typedef struct kTimer {
    struct dynListNode dynListPtrs; // hard-coded, don't touch
    hwTime_t value;
    hwTime_t interval;
    xm_u32_t flags;
#define KTIMER_ARMED (1<<0)
    void *actionArgs;
    void (*Action)(struct kTimer *, void *);
} kTimer_t;
```

### ./core/include/queue.h:23:struct queue

This struct is not in use.

### ./core/include/objdir.h:75:struct object

Really similar to Linux kernel object oriented programming.

```c
typedef xm_s32_t (*readObjOp_t)(xmObjDesc_t, void *, xmSize_t, xm_u32_t *);
typedef xm_s32_t (*writeObjOp_t)(xmObjDesc_t, void *, xmSize_t, xm_u32_t *);
typedef xm_s32_t (*seekObjOp_t)(xmObjDesc_t, xmSize_t, xm_u32_t);
typedef xm_s32_t (*ctrlObjOp_t)(xmObjDesc_t, xm_u32_t, void *);
struct object {
    readObjOp_t Read;
    writeObjOp_t Write;
    seekObjOp_t Seek;
    ctrlObjOp_t Ctrl;
};
```

### ./core/include/logstream.h:30:struct logStreamInfo
### ./core/include/logstream.h:33:struct logStreamCtrl
### ./core/include/logstream.h:27:struct logStreamHdr
```c
struct logStreamHdr {
#define LOGSTREAM_MAGIC1 0xF9E8D7C6
    xm_u32_t magic1;
    struct logStreamInfo {
        xm_s32_t elemSize, maxNoElem, lock, cHdr;
    } info;
    struct logStreamCtrl {
        xm_s32_t tail, elem, head, d;
    } ctrl[2];
#define LOGSTREAM_MAGIC2 0x1A2B3C4D
    xm_u32_t magic2;
};
```

### ./core/include/logstream.h:40:struct logStream

```c
struct logStream {
    struct logStreamInfo info;
    struct logStreamCtrl ctrl;
    spinLock_t lock;
    const kDevice_t *kDev;
};
```

./user/libxm/include/container.h:20:struct xefContainerFile

Container file; Container handler; file tab; partition tab; image address;
```c
struct xefContainerFile {
    struct xmefContainerHdr *hdr;
    struct xmefFile *fileTab;
    struct xmefPartition *partitionTab;
    xm_u8_t *image;
};
```

./user/libxm/include/xef.h:25:struct xefFile

```c
struct xefFile {
    struct xefHdr *hdr;
    struct xefSegment *segmentTab;
#if 0
#ifdef CONFIG_IA32
    struct xefRel *relTab;
    struct xefRela *relaTab;
#endif
#endif
    struct xefCustomFile *customFileTab;
    xm_u8_t *image;
};
```

./user/xal/include/sparcv8/irqs.h:42:typedef struct trapCtxt

The structure that passes register value during trap handling.

```c
typedef struct trapCtxt {
    xm_u32_t y;
    xm_u32_t g1;
    xm_u32_t g2;
    xm_u32_t g3;
    xm_u32_t g4;
    xm_u32_t g5;
    xm_u32_t g6;
    xm_u32_t g7;
    xm_u32_t nPc;
    xm_u32_t irqNr;
    xm_u32_t flags;
    xm_u32_t pc;
} trapCtxt_t;
```
