## 1

/core/include/x86/apic.h

```
#define LAPIC_TIMER_IRQ LAPIC_IRQ(0)
```
Could cause error because
```
#define LAPIC_IRQ(x) ((X) + CONFIG_MAX_NO_IOINT)
```
CONFIG_MAX_NO_IOINT could be 32. And it will cause index error in hwirqctrl array

## 2

```
static inline void DoPreemption(void) {

#ifndef CONFIG_SMP
	localSched_t *sched=GET_LOCAL_SCHED();
    localCpu_t *cpu=GET_LOCAL_CPU();
    HwIrqSetMask(cpu->globalIrqMask);
#endif
	HwSti();

    DoNop();

    HwCli();
#ifndef CONFIG_SMP
    HwIrqSetMask(sched->cKThread->ctrl.irqMask);
#endif
}
```

Must not disable all irq.