all: objs

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

SRCS_C-y = hypercalls.c irqs.c kthread.c leon_pic.c leon_timers.c panic.c processor.c setup.c

SRCS_S-y = entry.S head.S start.S

SRCS_C-$(CONFIG_MMU) += physmm.c vmmap.c

SRCS_C-$(CONFIG_SMP) += smp.c

SRCS=$(SRCS_S-y) $(SRCS_C-y)

OBJS := $(patsubst %.c,%.o,$(SRCS_C-y))
OBJS := $(patsubst %.S,%.o,$(SRCS_S-y)) $(OBJS)

objs: $(OBJS)
	$(TARGET_LD) $(TARGET_LDFLAGS) -r -o karch.o $(OBJS)
