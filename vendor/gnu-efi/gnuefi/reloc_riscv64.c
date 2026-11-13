#include <efi.h>

#include <elf.h>

#define Elf_Dyn		Elf64_Dyn
#define Elf_Rela	Elf64_Rela
#define ELF_R_TYPE	ELF64_R_TYPE

EFI_STATUS EFIAPI _relocate(long ldbase, Elf_Dyn *dyn)
{
	long relsz = 0, relent = 0;
	Elf_Rela *rel = NULL;
	unsigned long *addr;
	int i;

	for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
		switch (dyn[i].d_tag) {
		case DT_RELA:
			rel = (Elf_Rela *)((unsigned long)dyn[i].d_un.d_ptr + ldbase);
			break;
		case DT_RELASZ:
			relsz = dyn[i].d_un.d_val;
			break;
		case DT_RELAENT:
			relent = dyn[i].d_un.d_val;
			break;
		default:
			break;
		}
	}

	if (!rel && relent == 0)
		return EFI_SUCCESS;

	if (!rel || relent == 0)
		return EFI_LOAD_ERROR;

	while (relsz > 0) {

		switch (ELF_R_TYPE(rel->r_info)) {
		case R_RISCV_RELATIVE:
			addr = (unsigned long *)(ldbase + rel->r_offset);
			*addr = ldbase + rel->r_addend;
			break;
		default:
				break;
		}
		rel = (Elf_Rela *)((char *)rel + relent);
		relsz -= relent;
	}
	return EFI_SUCCESS;
}
