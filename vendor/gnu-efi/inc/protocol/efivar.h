#define VARIABLE_STORE_PROTOCOL    \
    { 0xf088cd91, 0xa046, 0x11d2, {0x8e, 0x42, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

INTERFACE_DECL(_EFI_VARIABLE_STORE);

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_CLEAR) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN OUT VOID                     *Scratch
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_READ) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    IN VOID                         *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_SIZE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        NoBanks
    );

typedef
EFI_STATUS
(EFIAPI *EFI_TRANSACTION_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN VOID                         *NewContents
    );

typedef struct _EFI_VARIABLE_STORE {

    UINT32                      Attributes;
    UINT32                      BankSize;
    UINT32                      NoBanks;

    EFI_STORE_CLEAR             ClearStore;
    EFI_STORE_READ              ReadStore;
    EFI_STORE_UPDATE            UpdateStore;
    EFI_STORE_SIZE              SizeStore OPTIONAL;
    EFI_TRANSACTION_UPDATE      TransactionUpdate OPTIONAL;

} EFI_VARIABLE_STORE;
