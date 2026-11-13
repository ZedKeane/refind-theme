SRCDIR = .

VPATH = $(SRCDIR)
TOPDIR = $(SRCDIR)/..

include $(SRCDIR)/../Make.defaults

PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig
FILES	= reloc_$(ARCH)

OBJS	= $(FILES:%=%.o)

reloc_aarch64.o: CFLAGS += -fno-jump-tables

TARGETS	= crt0-efi-$(ARCH).o libgnuefi.a

all:	$(TARGETS) gnu-efi.pc

libgnuefi.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

gnu-efi.pc:
	sed \
		-e 's:@PREFIX@:$(PREFIX):g' \
		-e 's:@EXEC_PREFIX@:$(EXEC_PREFIX):g' \
		-e 's:@INCLUDEDIR@:$(INCLUDEDIR):g' \
		-e 's:@LIBDIR@:$(LIBDIR):g' \
		-e 's:@VERSION@:$(VERSION):g' \
	$(SRCDIR)/gnu-efi.pc.in > gnu-efi.pc

clean:
	@rm -vf $(TARGETS) *~ *.o $(OBJS)

install:
	mkdir -p $(INSTALLROOT)$(LIBDIR)
	$(INSTALL) -m 644 $(TARGETS) $(INSTALLROOT)$(LIBDIR)
ifneq (,$(findstring FreeBSD,$(OS)))
 ifeq ($(ARCH),x86_64)
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_fbsd_efi.lds $(INSTALLROOT)$(LIBDIR)
 else
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_efi.lds $(INSTALLROOT)$(LIBDIR)
 endif
else
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_efi.lds $(INSTALLROOT)$(LIBDIR)
endif
	$(INSTALL) -d $(INSTALLROOT)$(PKGCONFIGDIR)
	$(INSTALL) -m 644 gnu-efi.pc $(INSTALLROOT)$(PKGCONFIGDIR)

include $(SRCDIR)/../Make.rules
