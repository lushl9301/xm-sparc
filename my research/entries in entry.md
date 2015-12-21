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