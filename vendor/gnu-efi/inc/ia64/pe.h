#ifndef _PE_H
#define _PE_H

#define IMAGE_DOS_SIGNATURE                 0x5A4D
#define IMAGE_OS2_SIGNATURE                 0x454E
#define IMAGE_OS2_SIGNATURE_LE              0x454C
#define IMAGE_NT_SIGNATURE                  0x00004550
#define IMAGE_EDOS_SIGNATURE                0x44454550

#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_IA64_DIR64            10

struct Plabel {
    UINT64  EntryPoint;
    UINT64  NewGP;
};

typedef struct _IMAGE_DOS_HEADER {
    UINT16   e_magic;
    UINT16   e_cblp;
    UINT16   e_cp;
    UINT16   e_crlc;
    UINT16   e_cparhdr;
    UINT16   e_minalloc;
    UINT16   e_maxalloc;
    UINT16   e_ss;
    UINT16   e_sp;
    UINT16   e_csum;
    UINT16   e_ip;
    UINT16   e_cs;
    UINT16   e_lfarlc;
    UINT16   e_ovno;
    UINT16   e_res[4];
    UINT16   e_oemid;
    UINT16   e_oeminfo;
    UINT16   e_res2[10];
    UINT32   e_lfanew;
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {
    UINT16   ne_magic;
    UINT8    ne_ver;
    UINT8    ne_rev;
    UINT16   ne_enttab;
    UINT16   ne_cbenttab;
    UINT32   ne_crc;
    UINT16   ne_flags;
    UINT16   ne_autodata;
    UINT16   ne_heap;
    UINT16   ne_stack;
    UINT32   ne_csip;
    UINT32   ne_sssp;
    UINT16   ne_cseg;
    UINT16   ne_cmod;
    UINT16   ne_cbnrestab;
    UINT16   ne_segtab;
    UINT16   ne_rsrctab;
    UINT16   ne_restab;
    UINT16   ne_modtab;
    UINT16   ne_imptab;
    UINT32   ne_nrestab;
    UINT16   ne_cmovent;
    UINT16   ne_align;
    UINT16   ne_cres;
    UINT8    ne_exetyp;
    UINT8    ne_flagsothers;
    UINT16   ne_pretthunks;
    UINT16   ne_psegrefbytes;
    UINT16   ne_swaparea;
    UINT16   ne_expver;
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    UINT16   Machine;
    UINT16   NumberOfSections;
    UINT32   TimeDateStamp;
    UINT32   PointerToSymbolTable;
    UINT32   NumberOfSymbols;
    UINT16   SizeOfOptionalHeader;
    UINT16   Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080
#define IMAGE_FILE_32BIT_MACHINE             0x0100
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200
#define IMAGE_FILE_SYSTEM                    0x1000
#define IMAGE_FILE_DLL                       0x2000
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x14c
#define IMAGE_FILE_MACHINE_R3000             0x162
#define IMAGE_FILE_MACHINE_R4000             0x166
#define IMAGE_FILE_MACHINE_ALPHA             0x184
#define IMAGE_FILE_MACHINE_ARMTHUMB_MIXED    0x1c2
#define IMAGE_FILE_MACHINE_POWERPC           0x1F0
#define IMAGE_FILE_MACHINE_IA64              0x200
#define IMAGE_FILE_MACHINE_TAHOE             0x7cc
#define IMAGE_FILE_MACHINE_EBC               0xebc
#define IMAGE_FILE_MACHINE_X64               0x8664

typedef struct _IMAGE_DATA_DIRECTORY {
    UINT32   VirtualAddress;
    UINT32   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    UINT16  Magic;
    UINT8   MajorLinkerVersion;
    UINT8   MinorLinkerVersion;
    UINT32  SizeOfCode;
    UINT32  SizeOfInitializedData;
    UINT32  SizeOfUninitializedData;
    UINT32  AddressOfEntryPoint;
    UINT32  BaseOfCode;
    UINT32  BaseOfData;
    UINT32  BaseOfBss;
    UINT32  GprMask;
    UINT32  CprMask[4];
    UINT32  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER {
    UINT16      Magic;
    UINT8       MajorLinkerVersion;
    UINT8       MinorLinkerVersion;
    UINT32      SizeOfCode;
    UINT32      SizeOfInitializedData;
    UINT32      SizeOfUninitializedData;
    UINT32      AddressOfEntryPoint;
    UINT32      BaseOfCode;

    UINT64      ImageBase;
    UINT32      SectionAlignment;
    UINT32      FileAlignment;
    UINT16      MajorOperatingSystemVersion;
    UINT16      MinorOperatingSystemVersion;
    UINT16      MajorImageVersion;
    UINT16      MinorImageVersion;
    UINT16      MajorSubsystemVersion;
    UINT16      MinorSubsystemVersion;
    UINT32      Win32VersionValue;
    UINT32      SizeOfImage;
    UINT32      SizeOfHeaders;
    UINT32      CheckSum;
    UINT16      Subsystem;
    UINT16      DllCharacteristics;
    UINT64      SizeOfStackReserve;
    UINT64      SizeOfStackCommit;
    UINT64      SizeOfHeapReserve;
    UINT64      SizeOfHeapCommit;
    UINT32      LoaderFlags;
    UINT32      NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER      224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    244

#define IMAGE_NT_OPTIONAL_HDR_MAGIC        0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

typedef struct _IMAGE_NT_HEADERS {
    UINT32 Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((UINT32)ntheader +                                                  \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))

#define IMAGE_SUBSYSTEM_UNKNOWN              0
#define IMAGE_SUBSYSTEM_NATIVE               1
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3
#define IMAGE_SUBSYSTEM_OS2_CUI              5
#define IMAGE_SUBSYSTEM_POSIX_CUI            7

#define IMAGE_DIRECTORY_ENTRY_EXPORT         0
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8
#define IMAGE_DIRECTORY_ENTRY_TLS            9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    UINT8   Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            UINT32   PhysicalAddress;
            UINT32   VirtualSize;
    } Misc;
    UINT32   VirtualAddress;
    UINT32   SizeOfRawData;
    UINT32   PointerToRawData;
    UINT32   PointerToRelocations;
    UINT32   PointerToLinenumbers;
    UINT16   NumberOfRelocations;
    UINT16   NumberOfLinenumbers;
    UINT32   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

#define IMAGE_SCN_TYPE_NO_PAD                0x00000008

#define IMAGE_SCN_CNT_CODE                   0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080

#define IMAGE_SCN_LNK_OTHER                  0x00000100
#define IMAGE_SCN_LNK_INFO                   0x00000200
#define IMAGE_SCN_LNK_REMOVE                 0x00000800
#define IMAGE_SCN_LNK_COMDAT                 0x00001000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000

#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000
#define IMAGE_SCN_MEM_SHARED                 0x10000000
#define IMAGE_SCN_MEM_EXECUTE                0x20000000
#define IMAGE_SCN_MEM_READ                   0x40000000
#define IMAGE_SCN_MEM_WRITE                  0x80000000

#define IMAGE_SIZEOF_SYMBOL                  18

#define IMAGE_SYM_UNDEFINED           (UINT16)0
#define IMAGE_SYM_ABSOLUTE            (UINT16)-1
#define IMAGE_SYM_DEBUG               (UINT16)-2

#define IMAGE_SYM_TYPE_NULL                  0
#define IMAGE_SYM_TYPE_VOID                  1
#define IMAGE_SYM_TYPE_CHAR                  2
#define IMAGE_SYM_TYPE_SHORT                 3
#define IMAGE_SYM_TYPE_INT                   4
#define IMAGE_SYM_TYPE_LONG                  5
#define IMAGE_SYM_TYPE_FLOAT                 6
#define IMAGE_SYM_TYPE_DOUBLE                7
#define IMAGE_SYM_TYPE_STRUCT                8
#define IMAGE_SYM_TYPE_UNION                 9
#define IMAGE_SYM_TYPE_ENUM                  10
#define IMAGE_SYM_TYPE_MOE                   11
#define IMAGE_SYM_TYPE_BYTE                  12
#define IMAGE_SYM_TYPE_WORD                  13
#define IMAGE_SYM_TYPE_UINT                  14
#define IMAGE_SYM_TYPE_DWORD                 15

#define IMAGE_SYM_DTYPE_NULL                 0
#define IMAGE_SYM_DTYPE_POINTER              1
#define IMAGE_SYM_DTYPE_FUNCTION             2
#define IMAGE_SYM_DTYPE_ARRAY                3

#define IMAGE_SYM_CLASS_END_OF_FUNCTION      (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                 0
#define IMAGE_SYM_CLASS_AUTOMATIC            1
#define IMAGE_SYM_CLASS_EXTERNAL             2
#define IMAGE_SYM_CLASS_STATIC               3
#define IMAGE_SYM_CLASS_REGISTER             4
#define IMAGE_SYM_CLASS_EXTERNAL_DEF         5
#define IMAGE_SYM_CLASS_LABEL                6
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL      7
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT     8
#define IMAGE_SYM_CLASS_ARGUMENT             9
#define IMAGE_SYM_CLASS_STRUCT_TAG           10
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION      11
#define IMAGE_SYM_CLASS_UNION_TAG            12
#define IMAGE_SYM_CLASS_TYPE_DEFINITION      13
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC     14
#define IMAGE_SYM_CLASS_ENUM_TAG             15
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM       16
#define IMAGE_SYM_CLASS_REGISTER_PARAM       17
#define IMAGE_SYM_CLASS_BIT_FIELD            18
#define IMAGE_SYM_CLASS_BLOCK                100
#define IMAGE_SYM_CLASS_FUNCTION             101
#define IMAGE_SYM_CLASS_END_OF_STRUCT        102
#define IMAGE_SYM_CLASS_FILE                 103

#define IMAGE_SYM_CLASS_SECTION              104
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL        105

#define N_BTMASK                            017
#define N_TMASK                             060
#define N_TMASK1                            0300
#define N_TMASK2                            0360
#define N_BTSHFT                            4
#define N_TSHIFT                            2

#define IMAGE_COMDAT_SELECT_NODUPLICATES   1
#define IMAGE_COMDAT_SELECT_ANY            2
#define IMAGE_COMDAT_SELECT_SAME_SIZE      3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH    4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE    5

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY 1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY   2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS     3

typedef struct _IMAGE_RELOCATION {
    UINT32   VirtualAddress;
    UINT32   SymbolTableIndex;
    UINT16   Type;
} IMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION              10

#define IMAGE_REL_I386_ABSOLUTE              0
#define IMAGE_REL_I386_DIR16                 01
#define IMAGE_REL_I386_REL16                 02
#define IMAGE_REL_I386_DIR32                 06
#define IMAGE_REL_I386_DIR32NB               07
#define IMAGE_REL_I386_SEG12                 011
#define IMAGE_REL_I386_SECTION               012
#define IMAGE_REL_I386_SECREL                013
#define IMAGE_REL_I386_REL32                 024

#define IMAGE_REL_MIPS_ABSOLUTE              0
#define IMAGE_REL_MIPS_REFHALF               01
#define IMAGE_REL_MIPS_REFWORD               02
#define IMAGE_REL_MIPS_JMPADDR               03
#define IMAGE_REL_MIPS_REFHI                 04
#define IMAGE_REL_MIPS_REFLO                 05
#define IMAGE_REL_MIPS_GPREL                 06
#define IMAGE_REL_MIPS_LITERAL               07
#define IMAGE_REL_MIPS_SECTION               012
#define IMAGE_REL_MIPS_SECREL                013
#define IMAGE_REL_MIPS_REFWORDNB             042
#define IMAGE_REL_MIPS_PAIR                  045

#define IMAGE_REL_ALPHA_ABSOLUTE             0x0
#define IMAGE_REL_ALPHA_REFLONG              0x1
#define IMAGE_REL_ALPHA_REFQUAD              0x2
#define IMAGE_REL_ALPHA_GPREL32              0x3
#define IMAGE_REL_ALPHA_LITERAL              0x4
#define IMAGE_REL_ALPHA_LITUSE               0x5
#define IMAGE_REL_ALPHA_GPDISP               0x6
#define IMAGE_REL_ALPHA_BRADDR               0x7
#define IMAGE_REL_ALPHA_HINT                 0x8
#define IMAGE_REL_ALPHA_INLINE_REFLONG       0x9
#define IMAGE_REL_ALPHA_REFHI                0xA
#define IMAGE_REL_ALPHA_REFLO                0xB
#define IMAGE_REL_ALPHA_PAIR                 0xC
#define IMAGE_REL_ALPHA_MATCH                0xD
#define IMAGE_REL_ALPHA_SECTION              0xE
#define IMAGE_REL_ALPHA_SECREL               0xF
#define IMAGE_REL_ALPHA_REFLONGNB            0x10

#define IMAGE_REL_PPC_ABSOLUTE 0x0000
#define IMAGE_REL_PPC_ADDR64   0x0001
#define IMAGE_REL_PPC_ADDR32   0x0002
#define IMAGE_REL_PPC_ADDR24   0x0003
#define IMAGE_REL_PPC_ADDR16   0x0004
#define IMAGE_REL_PPC_ADDR14   0x0005
#define IMAGE_REL_PPC_REL24    0x0006
#define IMAGE_REL_PPC_REL14    0x0007
#define IMAGE_REL_PPC_TOCREL16 0x0008
#define IMAGE_REL_PPC_TOCREL14 0x0009

#define IMAGE_REL_PPC_ADDR32NB 0x000A
#define IMAGE_REL_PPC_SECREL   0x000B
#define IMAGE_REL_PPC_SECTION  0x000C
#define IMAGE_REL_PPC_IFGLUE   0x000D
#define IMAGE_REL_PPC_IMGLUE   0x000E

#define IMAGE_REL_PPC_TYPEMASK 0x00FF

#define IMAGE_REL_PPC_NEG      0x0100
#define IMAGE_REL_PPC_BRTAKEN  0x0200
#define IMAGE_REL_PPC_BRNTAKEN 0x0400
#define IMAGE_REL_PPC_TOCDEFN  0x0800

typedef struct _IMAGE_BASE_RELOCATION {
    UINT32   VirtualAddress;
    UINT32   SizeOfBlock;

} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10

typedef struct _IMAGE_LINENUMBER {
    union {
        UINT32   SymbolTableIndex;
        UINT32   VirtualAddress;
    } Type;
    UINT16    Linenumber;
} IMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       "//              "

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    UINT8     Name[16];
    UINT8     Date[12];
    UINT8     UserID[6];
    UINT8     GroupID[6];
    UINT8     Mode[8];
    UINT8     Size[10];
    UINT8     EndHeader[2];
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

typedef struct _IMAGE_EXPORT_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16   MajorVersion;
    UINT16   MinorVersion;
    UINT32   Name;
    UINT32   Base;
    UINT32   NumberOfFunctions;
    UINT32   NumberOfNames;
    UINT32   AddressOfFunctions;
    UINT32   AddressOfNames;
    UINT32   AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_IMPORT_BY_NAME {
    UINT16    Hint;
    UINT8     Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA {
    union {
        UINT32 Function;
        UINT32 Ordinal;
        PIMAGE_IMPORT_BY_NAME AddressOfData;
    } u1;
} IMAGE_THUNK_DATA, *PIMAGE_THUNK_DATA;

#define IMAGE_ORDINAL_FLAG 0x80000000
#define IMAGE_SNAP_BY_ORDINAL(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG) != 0)
#define IMAGE_ORDINAL(Ordinal) (Ordinal & 0xffff)

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT32   ForwarderChain;
    UINT32   Name;
    PIMAGE_THUNK_DATA FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;

#endif
