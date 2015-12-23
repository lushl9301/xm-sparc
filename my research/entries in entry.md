### ENTRY WindowOverflowTrap
```
    if SV {
#if defined(CONFIG_MMU)
        if (%g2 < CONFIG_XM_OFFSET) {
            break;
        } else {
            CommonTrapHandler();
        }
    }
#endif
    //goto 3f
    // __fromWH
    %psr |= 0;

    save
    //TODO
    restore

    *(__fromWH) = 0;
    %psr = %l0;

    //JMPL %l1
    return %l2
}
```

### ENTRY WindowUnderflowTrap
```
    if SV {
#if defined(CONFIG_MMU)
        if (%g2 < CONFIG_XM_OFFSET) {
            break;
        } else {
            CommonTrapHandler();
        }
    }
#endif
    //goto 3f
    // __fromWH
    %psr |= 0;

    restore
    restore
    //TODO
    save
    save

    *(__fromWH) = 0;
    %psr = %l0;

    //JMPL %l1
    return %l2
}
```

### ENTRY FpDisabledTrap
```
%l5 = GET_CKTHREAD_FLAGS();
if (%l5 & (1<<1)) {
    %l5 = GET_CKTHREAD();
    %l6 = CKTHREAD_CTRL_GET_IFLAGS();
    %l5 |= 0x00001000;
    if (%l5 == %l6) {
        break; // cannot be set; or error here; goto handler
    }
    %l5 = %psr;
    %l6 |= 0x00001000;
    %l5 != %l6;
    %psr = %l5;
    WR_DELAY();

#ifdef CONFIG_SMP
    %l5 = localSchedInfo;
    %l7 = GET_CPUID() * 24 + %l5; //pointer shift
#else
    $l7 = localSchedInfo;
#endif
    %l6 = [%l7 + _FPUOWNER_OFFSET];
    %l5 = [%l7+_CKTHREAD_OFFSET];

    if (%l6 != 0 && %l5 != %l6) {
        %l5 = [%l6+_CTRL_OFFSET+_G_OFFSET];
        //STORE here
    }
    if (%l5 != %l6) {
        %l6 = [%l7+_CKTHREAD_OFFSET];
        //RESTORE here
    }
    if (%l6 != 0) {
        %l0 |= 0x00001000;
        %psr = %l0;

        jmpl %l1;
        return %l2;
    }
}
```

### KPreempt
```
.Tbegin_kpreempt:
    %sp = %fp - (MIN_STACK_FRAME+_CPUCTXT_SIZEOF);

    %l5 = %fp - _CPUCTXT_SIZEOF;
    SAVE_TRQCTXT(%l5); //save ctxt from %l5

    %g1 = %wim;

    %g3 = %g1 >> %l0;
    if (%g3 == 1) {
        %wim = 0;
        WR_DELAY();

        save;
        %g7 = %g1 >> (CONFIG_REGISTER_WINDOWS - 1);
        %g1 = MOD_SHIFT2RIGHT(%g1, %g7, CONFIG_REGISTER_WINDOWS));
        restore;

        %wim = %g1;
        WR_DELAY();
    }
    PSR_SET_ET_PIL();

    //call handler
    call %l4;
    %o0 = %fp - _CPUCTXT_SIZEOF;

    PSR_UNSET_ET();

    %wim = %l4;
    %l6 = %l0 + 1;
    %l6 += PSR_CWP_MASK;
    %l5 = %l4 >> %l6;
    if (%l5 == 1) {
        %l4 = MOD_SHIFT2LEFT(%l5, CONFIG_REGISTER_WINDOWS);
        %wim = %l4;
        WR_DELAY();

        restore;
        RESTORE_CWND(%sp);
        save
    }
    RESTORE_IRQCTXT(%fp - _CPUCTXT_SIZEOF);
    RESTORE_PSR();

.Tend_kpreempt:
    jmpl %l1;
    rett %l2;
}
```

### ENTRY CommonTrapHandler
```
.Tbegin_trap:
    %l0 = %psr;
    %l5 = [__fromWH];
    if (%l5 != 1) {
        if (%l0 & 0x40) {
            //supervisor mode;
        } else {
            goto KPreempt;//above
        }
    }
FromIRet:
    %l5 = GET_CKTHREAD_STACK();
    SAVE_IRQCTXT(%l5 - _CPUCTXT_SIZEOF)

    %g2 = %l6;

    %g1 = %wim;
    %wim = 0;

    %l6 = %g1;
    %g6 = %psr;
    %g4 = MOD_INTEGER2BITMAP(%g6, %g7, CONFIG_REGISTER_WINDOWS);
    %g3 = 0;
    if (!(%g4 & %g1)) {
        restore
//2:
        while (!(%g4 & %g1)) {
            %g2 -= 8;
            [%g2] = %sp;
            %g2 -= REG_WND_FRAME;
            %sp = g2;
            %g4 = MOD_INTEGER2BITMAP(%g6, %g7, CONFIG_REGISTER_WINDOWS);
            %g3++;
            restore
        }
    }

    //coming back to the original register window
    %g5 = %psr;
    %g5 = %g3 -%g6;
    %g6 &= CONFIG_REGISTER_WINDOWS - 1;
    %g5 = !(%g5 & (CONFIG_REGISTER_WINDOWS - 1))
    %psr = %g5 + %g6;

    WR_DELAY();
    %l7 = %g3;

    %g3 = %g1 >> %l0;
    if (%g3 == 1) {
        save;
        %g1 = MOD_SHIFT2RIGHT(%g7, CONFIG_REGISTER_WINDOWS);
        SAVE_CWND(%sp);
        restore;
    }

    %wim = %g1;
    WR_DELAY();
    %sp = %g2 - MIN_STACK_FRAME;
    PSR_SET_ET_PIL();
#ifdef CONFIG_AUDIT_EVENTS
    AuditAsmHCall();
    nop
#endif
    call %l4;
    %o0 = %l5 - _CPUCTXT_SIZEOF;
    RaisePendIrqs();
    %o0 = %l5 - _CPUCTXT_SIZEOF;

    %l4 = %o0;
    PSR_UNSET_ET();

    %g1 = %l6;
    %g2 = %l5 - _CPUCTXT_SIZEOF;

    %g3 = %l7;
    %g4 = %wim;

    %wim = 0;
    WR_DELAY();

    if (%g3 < 0) {
        %g3 = %g5;
        goto 4;
    }
    %g6 = %psr;
    %g1 = MOD_INTEGER2BITMAP(%g1, %g6, %g7, CONFIG_REGISTER_WINDOWS);

2:
    do {
        %g1 = MOD_SHIFT2LEFT(%g1, %g7, CONFIG_REGISTER_WINDOWS);
        if (%g1 & %g4) {
            //3;
            do {
                restore;
                %g2 -= 8;
                %g7 = [%g2];
                %g2 -= REG_WND_FRAME;
                %g5--;
                %sp = %g7;
            } while (%g5 > 0);
            break;//goto 4
        }
        restore;
        %g2 -= 8;
        %sp = [%g2];
        %g5--;
        %g2 -= REG_WND_FRAME;
    } while (%g5 > 0);

4:
    %g5 = %psr;
    %g6 = %g5 - %g3; //%g3 is counter
    %g6 &= CONFIG_REGISTER_WINDOWS-1;
    %g5 = !(%g5 & (CONFIG_REGISTER_WINDOWS - 1));
    %psr = %g5 + %g6;
    WR_DELAY();

    %wim = %l6;
    WR_DELAY();

    RESTORE_IRQCTXT(%l5 - _CPUCTXT_SIZEOF);

    if (%l4 != -1) {
        %l4 = %l3;
        EmulateTrapSv();
    }

    DO_FLUSH_CACHE();

    RESTORE_PSR(%l0);
.Tend_trap:
    jmpl %l1;
    return %l2;
}
```

### ENTRY DoHypercall
```
    %l4 = %o7;
    if (%i0 > NR_HYPERCALLS - 1) {
        goto 1;
    }
#ifdef CONFIG_AUDIT_EVENTS
    %o1 = %i1;
    %o2 = %i2;
    %o3 = %i3;
    %o4 = %i4;
    %o0 = %i0;
    AuditHCall();
#endif
    %g1 = hypercallsTab;
    %g2 = %i0 << 2;
    %g1 = [%g1 + %g2];

    if (%g1 == 1) {
        goto 1;
    }
    %o0 = %i1;
    %o1 = %i2;
    %o2 = %i3;
    %o3 = %i4;
    %o4 = %i5;
    call %g1;
    %i0 = %o0;
#ifdef CONFIG_AUDIT_EVENTS
    call AuditHCallRet
#endif
    goto 2;

1:
    %o0 = XM_UNKNOWN_HYPERCALL;

2:
    [%l5 - 4] = [%l5 - 16];
    [%l5 - 16] += 4;
    jmp %l4 + 4;

```

### ENTRY AsmHypercallHandler
```
if (%i0 <= NR_ASM_HYPERCALLS - 1) {
#ifdef CONFIG_AUDIT_EVENTS
    //TODO
#endif
    %l5 = auditAsmHCall;
    %l6 = %i0 >> 2;
    %l5 = [%l5 + %l6];
    if (%l5 != 0) {
        jmpl %l5;
    }
    DO_FLUSH_CACHE();
    RESTORE_PSR();
    jmp %l2;
    return %l2 + 4;
}
```

### ENTRY EmulateTrap
```
    %l3 = %l3 * 2 + 1;
    SET_CKTHREAD_SWTRAP(%l3, %l5, %l6);
    %l4 = _retl; //asm label
    %l5 = FromIRet;
    jmp %l5;

EmulateTrapSv:
    %l4, %l5 = CKTHREAD_CTRL_GET_IFLAGS();
    %l5 &= !PSR_ET_BIT; //clear bit
    CKTHREAD_CTRL_RESTORE_IFLAGS(%l4, %l5);

    %l4 = GET_CKTHREAD_STACK();
    %l5 = %g4;
    %g4 = %l4 - 0x20;
    PSR_SET_ET_PIL();
    restore();
    save();
    save();
    restore();
    %l4 = GET_CKTHREAD_STACK();
    RESTORE_REGRW(%l4 - 0x20);
    %g4 = %l5;
    PSR_UNSET_ET();

    %o0 = %l3;
    %o2 = %l0 & (!PSR_CWP_MASK);

    %l0 &=  (!(PSR_PIL_MASK|PSR_ET_BIT));

    %l5 = CKTHREAD_CTRL_GET_IFLAGS();
    %l0 |= %l4;

    save();

    DO_FLUSH_CACHE();
    %l0 = %psr;
    %l0 &= PSR_CWP_MASK;
    %psr = %i2 + %l0;

.Tend_etrap:
    jmp %i0;
    return %i0 + 4;
```

### ENTRY SparcIRetSys
```
restore();
```

### ENTRY SIRetCheckRetAddr
```
    PSR_SET_ET_PIL();
    if (%l1 <= CONFIG_XM_OFFSET && %l2 <= CONFIG_XM_OFFSET) {
        %l5 = [%l1];
        %l5 = [%l2];
        PSR_UNSET_ET_PIL();
    } else {
        save();
        %l3 = 0;
        CommonTrapHandler();
    }
```

### ENTRY EIRetCheckRetAddr
```
%l3 = %psr;
%l4 = %wim;
%l5 = %l3 + 1;
%l5 += CONFIG_REGISTER_WINDOWS - 1;
%l5 = %l4 >> %l5;

if (%l5 == 1) {
    save();
    %l3 = 0;
    CommandTrapHandler();
_retl:
    return;
}

%l3 &= !PSR_ICC_MASK;
%l0 &= PSR_ICC_MASK;

%l0 |= PSR_ICC_MASK;

%l3 = CKTHREAD_CTRL_GET_IFLAGS();
%l3 |= PSR_ET_BIT
CKTHREAD_CTRL_RESTORE_IFLAGS(%l3);

%l3 &= PSR_PIL_MASK;
if (%l3 & PSR_PIL_MASK) {
    DO_FLUSH_CACHE();
    RESTORE_PSR();
    jmp %l1;
    return %l2;
}
```

### ENTRY SparcFlushRegWinSys
```
    PSR_SET_ET_PIL();
    %l3 = %g3;
    %l4 = %g4;
    %l5 = %g5;
    %l6 = %g6;

    %g4 = %wim;
    %wim = 0;
    WR_DELAY();

    %g3 = CONFIG_REGISTER_WINDOWS - 2;

    %g5 = MOD_INTEGER2BITMAP(%g5, %l0, %g6, CONFIG_REGISTER_WINDOWS);
    if (%g5 != %g4) {
        goto 3;
    }
1:
    do {
        %g3--;
        save();
        %g5 = MOD_SHIFT2RIGHT(%g5, %g6, CONFIG_REGISTER_WINDOWS);
    } while (%g5 == %g4);

    if (%g3 == 0) {
        goto 4;
    }

    do {
        %g3--;
        save();
        SAVE_CWND(%sp);
    } while (%g3 != 0);

4:
    save();
    save();
    %g6 = %l0 & 2;
    %g6 += CONFIG_REGISTER_WINDOWS - 1;
    %wim = 1 << %g6;
    WR_DELAY();

    %g4 = %l4;
    %g5 = %l5;
    %g6 = %l6;
    %g3 = %l3;
    jmp %l7 + 4;
```

### ENTRY SparcGetPsrSys
```
    %l3 = PSR_PIL_MASK|PSR_ET_BIT|PSR_EF_BIT;
    %i0 = %l0 andnot %l3;

    %l4 = CKTHREAD_CTRL_GET_IFLAGS();

    %l4 &= %l3;
    %i0 |= %l4;

    return;
```

### ENTRY SparcSetPsrSys
```
%l4 = %i1 & PSR_ICC_MASK;
%l0 &= !PSR_ICC_MASK;
%l0 |= %l4;

%l3 = PSR_PIL_MASK|PSR_ET_BIT;
%l4 = GET_CKTHREAD_FLAGS();
if (%l4 & 2) {
    %l3 |= PSR_EF_BIT;
    if (!(%i1 & PSR_EF_BIT)) {
        %l0 &= !%l5;
        %l0 |= %l4;
    }
}
%l3 &= %i1;
CKTHREAD_CTRL_SET_IFLAGS(%l3);

if (%l3 & PSR_ET_BIT) {
    exit;
}
if (%l3 & PSR_PIL_BIT) {
    exit;
}

%l1 = %l2;
%l2 += 4;

%l4 = _retl;
%l5 = FromIRet; //CommonTrapHandler

jmp %l5;
```

### ENTRY SparcSetPilSys
```
%l5 = CKTHREAD_CTRL_GET_IFLAGS();

%l5 |= PSR_PIL_MASK;

CKTHREAD_CTRL_RESTORE_IFLAGS(%l5);

return;
```

### ENTRY SparcClearPilSys
```
%l5 = CKTHREAD_CTRL_GET_IFLAGS();
%l5 &= !PSR_PIL_MASK;
CKTHREAD_CTRL_RESTORE_IFLAGS(%l5);

%l1 = %l2;
%l2 += 4;

%l4 = _retl;
%l5 = FromIRet;
jmp %l5;
```

### ENTRY SparcCtrlWinFlowSys
```
%l5 = %wim;
%l6 = (%l0 & 2) & 0x1f;
%l4 = %l6 - CONFIG_REGISTER_WINDOWS;
if (%l4 >= 0) { //branc on Neg
    %l6 = %l4;
}
%l6 = %l5 >> %l6;
if (%l6 != 1) {
    return;
}

%l5 = %wim;
%l6 = %l5 << 1;
%l5 >>= CONFIG_REGISTER_WINDOWS - 1;
%wim = %l5 + %l6;
WR_DELAY();

restore();
restore();
RESTORE_CWND();
save();
save();
return;
```
