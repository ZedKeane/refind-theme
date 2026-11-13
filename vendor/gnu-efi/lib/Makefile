SRCDIR = .

VPATH = $(SRCDIR)
TOPDIR = $(SRCDIR)/..

include $(SRCDIR)/../Make.defaults

FILES = boxdraw smbios console crc data debug dpath \
        entry error event exit guid hand hw init lock \
        misc pause print sread str cmdline\
	runtime/rtlock runtime/efirtlib runtime/rtstr runtime/vm runtime/rtdata  \
	$(ARCH)/initplat $(ARCH)/math $(ARCH)/setjmp

ifeq ($(ARCH),ia64)
FILES += $(ARCH)/salpal $(ARCH)/palproc
endif

ifeq ($(ARCH),x86_64)
FILES += $(ARCH)/callwrap $(ARCH)/efi_stub
endif

ifeq ($(ARCH),arm)
FILES += $(ARCH)/uldiv $(ARCH)/ldivmod $(ARCH)/div $(ARCH)/llsl $(ARCH)/llsr \
	 $(ARCH)/mullu
endif

OBJS  = $(FILES:%=%.o) ctors.o

SUBDIRS = ia32 x86_64 ia64 aarch64 arm mips64el riscv64 loongarch64 runtime

LIBDIRINSTALL = $(INSTALLROOT)$(LIBDIR)

all: libsubdirs libefi.a

.PHONY: libsubdirs
libsubdirs:
	@set -e ; for sdir in $(SUBDIRS); do mkdir -p $$sdir; done

$(OBJS): libsubdirs

libefi.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	@rm -vf libefi.a *~ $(OBJS) */*.o

$(LIBDIRINSTALL):
	mkdir -p $@

$(LIBDIRINSTALL)/libefi.a: libefi.a | $(LIBDIRINSTALL)
	$(INSTALL) -m 644 $< $(dir $@)

install: $(LIBDIRINSTALL)/libefi.a

include $(SRCDIR)/../Make.rules
