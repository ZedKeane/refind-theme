#ifndef _SHELLINTERFACE_H_
#define _SHELLINTERFACE_H_

#define SHELL_INTERFACE_PROTOCOL_GUID \
  { \
    0x47c7b223, 0xc42a, 0x11d2, {0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} \
  }

typedef enum {
  ARG_NO_ATTRIB         = 0x0,
  ARG_IS_QUOTED         = 1<<0,
  ARG_PARTIALLY_QUOTED  = 1<<1,
  ARG_FIRST_HALF_QUOTED = 1<<2,
  ARG_FIRST_CHAR_IS_ESC = 1<<3
} EFI_SHELL_ARG_INFO_TYPES;

typedef struct _EFI_SHELL_ARG_INFO {
  UINT32  Attributes;
} EFI_SHELL_ARG_INFO;

typedef struct {

  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE *Info;

  CHAR16                    **Argv;
  UINTN                     Argc;

  CHAR16                    **RedirArgv;
  UINTN                     RedirArgc;

  EFI_FILE         *StdIn;
  EFI_FILE         *StdOut;
  EFI_FILE         *StdErr;

  EFI_SHELL_ARG_INFO        *ArgInfo;

  BOOLEAN                   EchoOn;
} EFI_SHELL_INTERFACE;

#endif
