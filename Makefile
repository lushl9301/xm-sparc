all: core | extract_info
# errata_check

include ../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk

ifdef CONFIG_MMU
	MM=mmu
else 
	MM=
endif

ifeq ($(XTRATUM_PATH)/core/.config, $(wildcard $(XTRATUM_PATH)/core/.config))
	exists_config := 1
endif

$(if $(exists_config), $(if $(MM),, $(error "MM variable is not defined")))

XEF=$(XTRATUM_PATH)/user/bin/xmeformat

$(XEF):
	make -C $(XTRATUM_PATH)/user/tools/xef install

xm.lds: kernel/$(ARCH)/xm.lds.in include/config.h
	@$(TARGET_CC) $(TARGET_ASFLAGS) -x assembler-with-cpp -E -P -I$(XM_CORE_PATH)/include kernel/$(ARCH)/xm.lds.in -o kernel/$(ARCH)/xm.lds

XMCORE_SUBDIRS=\
	kernel/$(ARCH)\
	kernel/$(MM)\
	kernel\
	klibc klibc/$(ARCH)\
	objects drivers\

KOBJS=\
	build_info.o\
	ldr.o\
	kernel/$(ARCH)/karch.o\
	kernel/$(MM)/kmm.o\
	kernel/kern.o\
	klibc/libc.o\
	klibc/$(ARCH)/arch_libc.o\
	objects/objects.o drivers/drivers.o\

links:
	@exec echo -e "\n> Target architecture: [$(ARCH)] ";
	@if [ ! -d $(XTRATUM_PATH)/core/include/arch ] ; then \
		ln -sf $(XTRATUM_PATH)/core/include/$(ARCH) $(XTRATUM_PATH)/core/include/arch; \
	fi

Kconfig.ver:
	@exec echo "# $@: autogenerated file, don't edit"		>  $@
	@exec echo "config ARCH"					>> $@
	@exec echo "	string"						>> $@
	@exec echo "	default \"$(ARCH)\""				>> $@

	@exec echo "" >> $@
	@exec echo "config KERNELVERSION"				>> $@
	@exec echo "	string"						>> $@
	@exec echo "	default \"$$XTRATUMVERSION\""			>> $@

	@exec echo "" >> $@
	@exec echo "config XM_VERSION" >> $@
	@exec echo "	int"						>> $@
	@exec echo "	default $$XM_VERSION"	>> $@
	@exec echo "config XM_SUBVERSION" >> $@
	@exec echo "	int"						>> $@
	@exec echo "	default $$XM_SUBVERSION"	>> $@
	@exec echo "config XM_REVISION" >> $@
	@exec echo "	int"						>> $@
	@exec echo "	default $$XM_REVISION"	>> $@

KCONFIG=kernel/$(ARCH)/Kconfig

user_links:
	@make -C $(XTRATUM_PATH)/user links

config: links Kconfig.ver
	@$(XTRATUM_PATH)/scripts/kconfig/conf $(KCONFIG)

oldconfig: links Kconfig.ver
	@$(XTRATUM_PATH)/scripts/kconfig/conf -o $(KCONFIG)

silentoldconfig: links Kconfig.ver
	@mkdir -p $(XTRATUM_PATH)/core/include/config
	@$(XTRATUM_PATH)/scripts/kconfig/conf -s $(KCONFIG)

menuconfig: links Kconfig.ver
	@$(XTRATUM_PATH)/scripts/kconfig/mconf $(KCONFIG)

$(defconfig-targets): links Kconfig.ver
	@$(XTRATUM_PATH)/scripts/kconfig/conf -D $(XTRATUM_PATH)/core/kernel/$(ARCH)/$@ $(KCONFIG)
defconfig: 

#$(XTRATUM_PATH)/core/include/autoconf.h: $(XTRATUM_PATH)/core/.config silentoldconfig

ldr/ldr.bin:
	@$(MAKE) -s -C ldr

core: ldr/ldr.bin $(XTRATUM_PATH)/core/include/autoconf.h xm.lds generate_offsets build.info user_links $(XEF)
	@$(XTRATUM_PATH)/scripts/gencomp.pl $(XTRATUM_PATH)/core/include/comp.h
	@exec echo -e "\n> Building XM Core";
	@for dir in $(XMCORE_SUBDIRS) ; do \
		echo "  - $$dir"	; \
		$(MAKE) -s -C $$dir all  ; \
		if [ "$$?" -ne 0 ]; then exit 1 ; fi \
	done
	@exec echo "" | $(TARGET_CC) $(TARGET_CFLAGS) -x c -c - -o build_info.o
	@$(TARGET_OBJCOPY) --add-section .build_info=build.info build_info.o
	@exec echo "" | $(TARGET_CC) $(TARGET_CFLAGS) -x c -c - -o ldr.o
	@$(TARGET_OBJCOPY) --add-section .ldr=ldr/ldr.bin --set-section-flags .ldr=alloc ldr.o
	@exec echo -e "> Linking XM Core";
	@$(TARGET_LD) $(TARGET_LDFLAGS) -nostdlib -Tkernel/$(ARCH)/xm.lds -o xm_core $(KOBJS) $(LIBGCC) ;
	@$(TARGET_SIZE) xm_core;
ifdef CONFIG_XEF_COMPRESSION
	@$(XEF) build xm_core -c -o xm_core.xef
else
	@$(XEF) build xm_core -o xm_core.xef
endif
	@chmod -x xm_core
	@exec echo "> Done";

build.info:
	@exec echo "BUILD_TARGET_CC=\"`($(TARGET_CC) --version | head -1)`\"" > build.info;
	@exec echo "BUILD_TIME=\"`(LANG="C" date)`\"" >> build.info;
	@exec echo "BUILD_HOST=\"`(hostname)`\"" >> build.info;
	@exec echo "BUILD_UID=\"`(id -nu)`\"" >> build.info;
	@grep ^CONFIG_* .config >> build.info;

generate_offsets:
	@$(TARGET_CC) $(TARGET_CFLAGS) -S -o offsets.S $(XTRATUM_PATH)/scripts/asm-offsets.c -D_GENERATE_OFFSETS_ -D_OFFS_FILE_=\"$(ARCH)/gen_offsets.h\"
	@$(SHELL) $(XTRATUM_PATH)/scripts/asm-offsets.sh offsets.h < offsets.S > $(XM_CORE_PATH)/include/$(ARCH)/asm_offsets.h
	@$(RM) -f offsets.S

extract_info:
	@$(HOST_CC) -Wall -O2 -o $(XTRATUM_PATH)/scripts/extractinfo $(XTRATUM_PATH)/scripts/extractinfo.c -DTARGET_OBJCOPY=\"$(TARGET_OBJCOPY)\" --include $(XTRATUM_PATH)/core/include/autoconf.h
	@$(XTRATUM_PATH)/scripts/extractinfo $(XTRATUM_PATH)/core/xm_core 2> $(XM_CORE_PATH)/include/$(ARCH)/ginfo.h

errata_check:
	@if ! $(XTRATUM_PATH)/scripts/erratack.pl > /dev/null; then $(XTRATUM_PATH)/scripts/erratack.pl; fi

clean:
	@exec echo -e "> Cleaning XM Core";
	@exec echo -e "  - Removing *.o *.a *~ files";
	@find -name "*~" -exec rm '{}' \;
	@find -name "*.o" -exec rm '{}' \;
	@find -name "*.xo" -exec rm '{}' \;
	@find -name "*.a" -exec rm '{}' \;
	@$(RM) -f kernel/$(ARCH)/xm.lds include/$(ARCH)/asm_offsets.h include/$(ARCH)/ginfo.h include/$(ARCH)/brksize.h build.info xm_core xm_core.bin xm_core.xef ldr/*.elf ldr/*.bin ldr/*.lds
	@exec echo -e "> Done";
