SRCDIR = .

VPATH = $(SRCDIR)

ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

TARGET	= refind.efi

ifeq ($(ARCH),ia32)
  LIBEG = build32
  TARGET = refind_ia32.efi
endif

ifeq ($(ARCH),x86_64)
  LIBEG = build64
  TARGET = refind_x64.efi
endif

ifeq ($(ARCH),aarch64)
  LIBEG = build
  TARGET = refind_aa64.efi
endif

LOCAL_GNUEFI_CFLAGS  = -I$(SRCDIR) -I$(SRCDIR)/../include \
                       -I$(SRCDIR)/../libeg -I$(SRCDIR)/../mok
LOCAL_LDFLAGS   = -L$(SRCDIR)/../libeg/ -L$(SRCDIR)/../mok/ \
                  -L$(SRCDIR)/../EfiLib/ -L$(SRCDIR)/../gzip
LOCAL_LIBS      = -leg -lmok -lEfiLib -lgzip

OBJS            = apple.o config.o crc32.o driver_support.o gpt.o icns.o \
                  install.o launch_efi.o launch_legacy.o lib.o line_edit.o \
                  linux.o log.o main.o menu.o mystrings.o pointer.o scan.o \
                  screen.o os_registry.o disk_name.o ui_language.o

include $(SRCDIR)/../Make.common

all: $(TARGET)

$(SHLIB_TARGET): $(OBJS)
	$(LD) $(LOCAL_LDFLAGS) $(GNUEFI_LDFLAGS) $(SUBSYSTEM_LDFLAG) $(OBJS) \
	      -o $@ $(LOCAL_LIBS) $(GNUEFI_LIBS)

$(TARGET): $(SHLIB_TARGET)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rodata \
		   -j .rel -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* \
		   -j .reloc --strip-unneeded $(FORMAT) $< $@
ifneq ($(OMIT_SBAT), 1)
	    $(OBJCOPY) --add-section .sbat=$(SRCDIR)/../$(REFIND_SBAT_CSV) \
		       --adjust-section-vma .sbat+10000000 $@
endif
	chmod a-x $(TARGET)

clean:
	rm -f $(TARGET) *~ *.so $(OBJS) *.efi *.obj refind_*.txt \
		refind_*.dll *.lib

os_registry.o: ../../../src/runtime/os_registry.c ../../../src/runtime/os_registry.h ../../../src/generated/os_registry_data.h
	$(CC) $(CFLAGS) $(GNUEFI_CFLAGS) $(LOCAL_GNUEFI_CFLAGS) -c $< -o $@

disk_name.o: ../../../src/runtime/disk_name.c ../../../src/runtime/disk_name.h
	$(CC) $(CFLAGS) $(GNUEFI_CFLAGS) $(LOCAL_GNUEFI_CFLAGS) -c $< -o $@

ui_language.o: ../../../src/runtime/ui_language.c ../../../src/runtime/ui_language.h ../../../src/generated/ui_language_data.h ../../../src/generated/ui_language_data.inc
	$(CC) $(CFLAGS) $(GNUEFI_CFLAGS) $(LOCAL_GNUEFI_CFLAGS) -c $< -o $@
