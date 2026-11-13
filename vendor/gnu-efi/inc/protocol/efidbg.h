#ifndef _EFIDBG_H_
#define _EFIDBG_H_

#include "eficontext.h"
#include "efiser.h"

typedef struct _DEBUGPORT_16550_CONFIG_DATA {
        UINT32							PortAddress;
        UINT64                          BaudRate;
    	UINT32               			ReceiveFifoDepth;
    	UINT32               			Timeout;
        UINT8                           Parity;
        UINT8                           DataBits;
        UINT8                           StopBits;
	    UINT32                       	ControlMask;
        BOOLEAN							RtsCtsEnable;
} DEBUGPORT_16550_CONFIG_DATA;

typedef struct _DEBUGPORT_16550_DEVICE_PATH {
        EFI_DEVICE_PATH                 Header;
        DEBUGPORT_16550_CONFIG_DATA		ConfigData;
} DEBUGPORT_16550_DEVICE_PATH;

typedef union {
    EFI_DEVICE_PATH                     DevPath;
    DEBUGPORT_16550_DEVICE_PATH         Uart;

} DEBUGPORT_DEV_PATH;

#define DEBUG_SUPPORT_PROTOCOL \
{ 0x2755590C, 0x6F3C, 0x42fa, 0x9E, 0xA4, 0xA3, 0xBA, 0x54, 0x3C, 0xDA, 0x25 }

typedef UINTN EXCEPTION_TYPE;

typedef
VOID
(*EXCEPTION_HANDLER) (
	IN EXCEPTION_TYPE ExceptionType,
    IN SYSTEM_CONTEXT *SystemContext
    );

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_TIMER_TICK_CALLBACK) (
    IN struct _EFI_DEBUG_SUPPORT_INTERFACE  *This,
    IN EXCEPTION_HANDLER	                TimerTickCallback
    );

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_EXCEPTION_HANDLER) (
    IN     struct _EFI_DEBUG_SUPPORT_INTERFACE  *This,
    IN     EXCEPTION_HANDLER                    ExceptionHandler,
    IN     EXCEPTION_TYPE                       ExceptionType
    );

typedef
EFI_STATUS
(EFIAPI *EFI_IP_CALL_TRACE) (
    IN     struct _EFI_DEBUG_SUPPORT_INTERFACE  *This
    );

#define EFI_DEBUG_SUPPORT_INTERFACE_REVISION     0x00010000

typedef struct _EFI_DEBUG_SUPPORT_INTERFACE {
    UINT32                          	Revision;
    EFI_REGISTER_TIMER_TICK_CALLBACK	RegisterTimerTickCallback;
    EFI_REGISTER_EXCEPTION_HANDLER  	RegisterExceptionHandler;
    EFI_IP_CALL_TRACE               	IpCallTrace;
} EFI_DEBUG_SUPPORT_INTERFACE;

#define DEBUGPORT_IO_PROTOCOL \
{ 0XEBA4E8D2, 0X3858, 0X41EC, 0XA2, 0X81, 0X26, 0X47, 0XBA, 0X96, 0X60, 0XD0 }

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_RESET) (
    IN struct _EFI_DEBUGPORT_IO_INTERFACE  	*This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_READ) (
    IN     struct _EFI_DEBUGPORT_IO_INTERFACE	*This,
    IN OUT UINTN                    		*BufferSize,
    OUT VOID                         		*Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_WRITE) (
    IN     struct _EFI_DEBUGPORT_IO_INTERFACE *This,
    IN OUT UINTN                    		*BufferSize,
    IN VOID                         		*Buffer
    );

#define EFI_DEBUGPORT_IO_INTERFACE_REVISION   0x00010000

typedef struct _EFI_DEBUGPORT_IO_INTERFACE {
    UINT32                          		Revision;
    EFI_DEBUGPORT_IO_READ					Read;
    EFI_DEBUGPORT_IO_WRITE					Write;
    EFI_DEBUGPORT_IO_RESET					Reset;
} EFI_DEBUGPORT_IO_INTERFACE;

#define DEBUGPORT_UART16550_CONTROL_PROTOCOL \
{ 0X628EA978, 0X4C26, 0X4605, 0XBC, 0X2, 0XA4, 0X2A, 0X49, 0X69, 0X17, 0XDD }

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_SET_ATTRIBUTES) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE  	*This,
    IN UINT64                       	BaudRate,
    IN UINT32                       	ReceiveFifoDepth,
    IN UINT32                       	Timeout,
    IN EFI_PARITY_TYPE       			Parity,
    IN UINT8                        	DataBits,
    IN EFI_STOP_BITS_TYPE    			StopBits
    );

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_SET_CONTROL_BITS) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE  	*This,
    IN UINT32                       	Control
    );

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_GET_CONTROL_BITS) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE	*This,
    OUT UINT32                      	*Control
    );

#define EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE_REVISION   0x00010000

typedef struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE {
    UINT32                          	Revision;
	EFI_UART16550_SET_ATTRIBUTES		SetAttributes;
	EFI_UART16550_SET_CONTROL_BITS		SetControl;
	EFI_UART16550_GET_CONTROL_BITS 		GetControl;
	DEBUGPORT_16550_CONFIG_DATA			*Mode;
} EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE;

#define DEVICE_PATH_DEBUGPORT DEBUGPORT_IO_PROTOCOL

#endif
