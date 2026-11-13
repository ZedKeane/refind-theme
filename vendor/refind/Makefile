include Make.common

SHELL=/bin/bash
LOADER_DIR=refind
FS_DIR=filesystems
LIBEG_DIR=libeg
MOK_DIR=mok
GZIP_DIR=gzip
GPTSYNC_DIR=gptsync
EFILIB_DIR=EfiLib

export TIANOBASE=/usr/local/UDK2014/MyWorkSpace
export EDK2BASE=/usr/local/edk2-vUDK2018

-include $(EDK2BASE)/Conf/target.txt
THISDIR=$(shell pwd)
EDK2_BUILDLOC=$(EDK2BASE)/Build/Refind/$(TARGET)_$(TOOL_CHAIN_TAG)/$(UC_ARCH)
EDK2_PROGRAM_BASENAMES=refind gptsync
EDK2_PROGRAMS=$(EDK2_PROGRAM_BASENAMES:=.efi)
EDK2_DRIVER_BASENAMES=btrfs ext4 ext2 hfs iso9660 reiserfs
EDK2_DRIVERS=$(EDK2_DRIVER_BASENAMES:=.efi)
EDK2_ALL_BASENAMES=$(EDK2_PROGRAM_BASENAMES) $(EDK2_DRIVER_BASENAMES)
EDK2_ALL_FILES=$(EDK2_ALL_BASENAMES:=.efi)

all:
ifneq ($(wildcard $(EDK2BASE)/*),)
	@echo "Found $(EDK2BASE); building with TianoCore EDK2"
	+make edk2
else ifneq ($(wildcard $(TIANOBASE)/*),)
	@echo "Found $(TIANOBASE); building with TianoCore UDK2014"
	+make tiano
else
	@echo "Did not find $(EDK2BASE) or $(TIANOBASE); building with GNU-EFI"
	+make gnuefi
endif

fs:
ifneq ($(wildcard $(EDK2BASE)/*),)
	@echo "Found $(EDK2BASE); building with TianoCore EDK2"
	+make fs_edk2
else ifneq ($(wildcard $(TIANOBASE)/*),)
	@echo "Found $(TIANOBASE); building with TianoCore UDK2014"
	+make fs_tiano
else
	@echo "Did not find $(EDK2BASE) or $(TIANOBASE); building with GNU-EFI"
	+make fs_gnuefi
endif

gptsync:
ifneq ($(wildcard $(EDK2BASE)/*),)
	@echo "Found $(EDK2BASE); building with TianoCore EDK2"
	+make gptsync_edk2
else ifneq ($(wildcard $(TIANOBASE)/*),)
	@echo "Found $(TIANOBASE); building with TianoCore UDK2014"
	+make gptsync_tiano
else
	@echo "Did not find $(EDK2BASE) or $(TIANOBASE); building with GNU-EFI"
	+make gptsync_gnuefi
endif

gnuefi:
	+make MAKEWITH=GNUEFI -C $(LIBEG_DIR)
	+make MAKEWITH=GNUEFI -C $(MOK_DIR)
	+make MAKEWITH=GNUEFI -C $(GZIP_DIR)
	+make MAKEWITH=GNUEFI -C $(EFILIB_DIR)
	+make MAKEWITH=GNUEFI -C $(LOADER_DIR)
	+make MAKEWITH=GNUEFI -C $(GPTSYNC_DIR) gnuefi

all_gnuefi: gnuefi fs_gnuefi

gptsync_gnuefi:
	+make MAKEWITH=GNUEFI -C $(GPTSYNC_DIR) gnuefi

fs_gnuefi:
	+make MAKEWITH=GNUEFI -C $(FS_DIR) all_gnuefi

tiano:
	-include $(TIANOBASE)/Conf/target.txt
	+make MAKEWITH=TIANO AR_TARGET=EfiLib -C $(EFILIB_DIR) -f Make.tiano
	+make MAKEWITH=TIANO AR_TARGET=libeg -C $(LIBEG_DIR) -f Make.tiano
	+make MAKEWITH=TIANO AR_TARGET=mok -C $(MOK_DIR) -f Make.tiano
	+make MAKEWITH=TIANO AR_TARGET=gzip -C $(GZIP_DIR) -f Make.tiano
	+make MAKEWITH=TIANO BUILDME=refind DLL_TARGET=refind -C $(LOADER_DIR) -f Make.tiano
ifneq ($(ARCH),aarch64)
	+make MAKEWITH=TIANO -C $(GPTSYNC_DIR) -f Make.tiano
endif

all_tiano: tiano fs_tiano

gptsync_tiano:
	-include $(TIANOBASE)/Conf/target.txt
	+make MAKEWITH=TIANO -C $(GPTSYNC_DIR) -f Make.tiano

fs_tiano:
	-include $(TIANOBASE)/Conf/target.txt
	+make MAKEWITH=TIANO -C $(FS_DIR)

edk2: build_edk2
	cp $(EDK2_BUILDLOC)/refind.efi ./refind/refind_$(FILENAME_CODE).efi
	cp $(EDK2_BUILDLOC)/gptsync.efi ./gptsync/gptsync_$(FILENAME_CODE).efi
ifneq ($(OMIT_SBAT), 1)
	$(OBJCOPY) --set-section-alignment '.sbat=512' --add-section .sbat=$(REFIND_SBAT_CSV) \
		--adjust-section-vma .sbat+10000000 ./refind/refind_$(FILENAME_CODE).efi
	$(OBJCOPY) --set-section-alignment '.sbat=512' --add-section .sbat=$(REFIND_SBAT_CSV) \
		--adjust-section-vma .sbat+10000000 ./gptsync/gptsync_$(FILENAME_CODE).efi
endif

all_edk2: build_edk2 fs_edk2
	cp $(EDK2_BUILDLOC)/refind.efi ./refind/refind_$(FILENAME_CODE).efi
	cp $(EDK2_BUILDLOC)/gptsync.efi ./gptsync/gptsync_$(FILENAME_CODE).efi

gptsync_edk2: build_edk2
	cp $(EDK2_BUILDLOC)/gptsync.efi ./gptsync/gptsync_$(FILENAME_CODE).efi

fs_edk2: build_edk2
ifeq ($(OMIT_SBAT), 1)
	for BASENAME in $(EDK2_DRIVER_BASENAMES) ; do \
		echo "Copying $$BASENAME""_$(FILENAME_CODE).efi" ; \
		cp "$(EDK2_BUILDLOC)/$$BASENAME.efi" ./drivers_$(FILENAME_CODE)/$$BASENAME\_$(FILENAME_CODE).efi ; \
	done
else
	for BASENAME in $(EDK2_DRIVER_BASENAMES) ; do \
		echo "Copying $$BASENAME""_$(FILENAME_CODE).efi" ; \
		cp "$(EDK2_BUILDLOC)/$$BASENAME.efi" ./drivers_$(FILENAME_CODE)/$$BASENAME\_$(FILENAME_CODE).efi ; \
		$(OBJCOPY) --set-section-alignment '.sbat=512' --add-section .sbat=$(REFIND_SBAT_CSV) \
		    --adjust-section-vma .sbat+10000000 ./drivers_$(FILENAME_CODE)/$$BASENAME\_$(FILENAME_CODE).efi ; \
	done
endif

build_edk2: $(EDK2BASE)/RefindPkg
	cd $(EDK2BASE) && \
	. ./edksetup.sh BaseTools && \
	build -a $(UC_ARCH) -p RefindPkg/RefindPkg.dsc
	mkdir -p ./drivers_$(FILENAME_CODE)

$(EDK2BASE)/RefindPkg:
	ln -s $(THISDIR) $(EDK2BASE)/RefindPkg

clean:
	make -C $(LIBEG_DIR) clean
	make -C $(MOK_DIR) clean
	make -C $(GZIP_DIR) clean
	make -C $(LOADER_DIR) clean
	make -C $(EFILIB_DIR) clean
	make -C $(FS_DIR) clean
	make -C $(GPTSYNC_DIR) clean
	rm -f include/*~
	rm -rf $(EDK2BASE)/Build/Refind
	rm -rf drivers_$(FILENAME_CODE)/*
	[ ! -L $(EDK2BASE)/RefindPkg ] || rm -v $(EDK2BASE)/RefindPkg

install:
	./refind-install
