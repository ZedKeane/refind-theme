#include "lib.h"

#include "efiprot.h"
#include "efishell.h"
#include "efishellintf.h"

#ifndef MAX_ARGV_CONTENTS_SIZE
# define MAX_CMDLINE_SIZE 1024
#endif
#ifndef MAX_ARGC
# define MAX_CMDLINE_ARGC 32
#endif

static
INTN
GetShellArgcArgvFromLoadedImage(
    EFI_HANDLE ImageHandle,
    CHAR16 **ResultArgv[]
    )
{
  EFI_STATUS Status;
  void *LoadedImage = NULL;
  static CHAR16 ArgvContents[MAX_CMDLINE_SIZE];
  static CHAR16 *Argv[MAX_CMDLINE_ARGC], *ArgStart, *c;
  UINTN Argc = 0, BufLen;

  Status = uefi_call_wrapper(BS->OpenProtocol, 6,
                             ImageHandle,
                             &LoadedImageProtocol,
                             &LoadedImage,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL
                             );
  if (EFI_ERROR(Status))
    return -1;

  BufLen = ((EFI_LOADED_IMAGE *)LoadedImage)->LoadOptionsSize;
  if (BufLen < 2)
    return -1;
  else if (BufLen > sizeof(ArgvContents))
    BufLen = sizeof(ArgvContents);

  CopyMem(ArgvContents, ((EFI_LOADED_IMAGE *)LoadedImage)->LoadOptions, BufLen);
  ArgvContents[MAX_CMDLINE_SIZE - 1] = L'\0';

  for (c = ArgStart = ArgvContents ; *c != L'\0' ; ++c) {
    if (*c == L' ') {
      *c = L'\0';
      if (Argc < MAX_CMDLINE_ARGC) Argv[Argc++] = ArgStart;
      ArgStart = c + 1;
    }
  }

  if ((*ArgStart != L'\0') && (Argc < MAX_CMDLINE_ARGC))
    Argv[Argc++] = ArgStart;

  *ResultArgv = Argv;
  return Argc;
}

INTN GetShellArgcArgv(EFI_HANDLE ImageHandle, CHAR16 **Argv[])
{

  EFI_STATUS Status;
  static const EFI_GUID ShellInterfaceProtocolGuid
      = SHELL_INTERFACE_PROTOCOL_GUID;
  EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol = NULL;
  EFI_SHELL_INTERFACE *EfiShellInterfaceProtocol = NULL;

  Status = uefi_call_wrapper(BS->OpenProtocol, 6,
                             ImageHandle,
                             (EFI_GUID*)&ShellParametersProtocolGuid,
                             (VOID **)&EfiShellParametersProtocol,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL
                             );
  if (!EFI_ERROR(Status))
  {

    *Argv = EfiShellParametersProtocol->Argv;
    return EfiShellParametersProtocol->Argc;
  }

  Status = uefi_call_wrapper(BS->OpenProtocol, 6,
                             ImageHandle,
                             (EFI_GUID*)&ShellInterfaceProtocolGuid,
                             (VOID **)&EfiShellInterfaceProtocol,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL
                             );
  if (!EFI_ERROR(Status))
  {

    *Argv = EfiShellInterfaceProtocol->Argv;
    return EfiShellInterfaceProtocol->Argc;
  }

  return GetShellArgcArgvFromLoadedImage(ImageHandle, Argv);
}
