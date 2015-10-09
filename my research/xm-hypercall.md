#XtratuM Hypercall Mechanisim Studies
##Case study of hypercall XM_get_time

###Hypercall Declaration

```c
extern __stdcall xm_s32_t XM_get_time(xm_u32_t clock_id, xmTime_t *time);
```

The hypercall is declared in user/libxm/include/xmhypercalls.h. We can see that the following list of hypercalls are declared in the header file :
	* Time management hypercalls
	* Partition status hypercalls
	* System status hypercalls
	* Object related hypercalls (read / write / seek / ctrl object)
	* Paging hypercalls
	* Hw interrupt management
These hypercalls are ```__stdcall``` as shown in the hypercall declaration.

```__stdcall``` is defined in core/include/arch/linkage.h. But this macro is simply EMPTY. Maybe, there will be future improvements.

These hypercalls are directly used by bare-metal application in XtratuM partitions. These hypercalls are the application interfaces. We will analyze these ```__stdcall`` hypercalls in this report.

###How to find hypercall implementation

There is no obvious implementation of ```XM_get_time```. It is assembled by macro. So how could we find the real implementation? And How are them linked?

#### Function Implementation

The implementations of functions are allocated in core folder. The function that is related to ```XM_get_time``` is found at core/kernel/hypercalls.c
```c
__hypercall xm_s32_t GetTimeSys(xm_u32_t clockId, xmTime_t *__gParam time) {
    localSched_t *sched=GET_LOCAL_SCHED();
    if (CheckGParam(time, sizeof(xm_s64_t), 8, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;
    switch(clockId) {
    case XM_HW_CLOCK:
        *time=GetSysClockUsec();
        break;
    case XM_EXEC_CLOCK:
        *time=GetTimeUsecVClock(&sched->cKThread->ctrl.g->vClock);
        break;
    default:
        return XM_INVALID_PARAM;
    }
    return XM_OK;
}
```

This function implementation suits the ```XM_get_time``` hypercall interface. But they have different function name, where is the linking macro?

#### Linking Macro

We tried to use the following bash command to find the hypercall implementation.

```bash
grep -ri "XM_get_time" .
```

However, what we get are only hypercall declaration and invoking from given bare-metal application examples.

```bash
grep -ri "_get_time" .
```

This command will give us one more piece of information:
```
./core/include/arch/hypercalls.h:#define __GET_TIME_NR 9
./core/include/hypercalls.h:#define get_time_nr __GET_TIME_NR
```

```__GET_TIMER_NR == 9``` should be the hypercall id. 

```bash
grep -ri "get_time" .
```

This command gives us an important line of code:

```
./user/libxm/sparcv8/hypercalls.c:xm_hcall2r(get_time, xm_u32_t, clock_id, xmTime_t *, time);
```
And this function is shown as follow:

```c
#define xm_hcall2(_hc, _t0, _a0, _t1, _a1) \
ASMLINK void XM_##_hc(_t0 _a0, _t1 _a1) { \
    xm_s32_t _r ; \
    _XM_HCALL2(_a0, _a1, _hc##_nr, _r); \
}

#define _XM_HCALL2(a0, a1, _hc_nr, _r) \
    __asm__ __volatile__ ("mov %0, %%o0\n\t" \
                          "mov %2, %%o1\n\t" \
                          "mov %3, %%o2\n\t" \
                          __DO_XMHC \
                          "mov %%o0, %0\n\t" : "=r" (_r) : "0" (_hc_nr), "r" (a0), "r" (a1) : \
                          "o0", "o1", "o2", "o3", "o4", "o5",       "o7")

#define __DO_XMHC "ta "TO_STR(XM_HYPERCALL_TRAP)"\n\t"
```


```_t0```, ```_t1``` are the type of 2 parameters. ```_a0```, ```_a1``` are the parameters. ```_hc``` is the name of hypercall passed in.

XtratuM uses ```XM_##_hc``` to assemble a new function. In this new function ```XM_get_time```. XtratuM just simply calls ```_XM_HCALL2```.

The 3rd arguments passed to ```_XM_HCALL2``` is ```_hc##_nr```, in this case ```get_time_nr``` which is exactly ```== __GET_TIME_NR == 9```. And ```_r ``` is the return value.

In the assembly code, we input ```a0 == %2```, and ```a1 == %3``` to register. And we pass in return variable and get the return value.

```__DO_XMHC``` after pre-process, it will be "ta 0xF0". And it is found that there is "./core/kernel/arch/start.S:220:	BUILD_HYPERCALL 0xF0", which is the hypercall section of XtratuM.

```"0" (_hc_nr)``` means passing ```__GET_TIME_NR == 9

From this macro we can see that ```XM_get_time``` is assembled by calling the corresponding hypercall function ```GetTimeSys``` using assembly code and hypercall id.