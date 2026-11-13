#ifndef _EFI_PXE_H
#define _EFI_PXE_H

#pragma pack(1)

#define PXE_INTEL_ORDER         1

#define PXE_UINT64_SUPPORT          1

#define PXE_BUSTYPE(a,b,c,d)            \
((((PXE_UINT32)(d) & 0xFF) << 24) | \
(((PXE_UINT32)(c) & 0xFF) << 16) |  \
(((PXE_UINT32)(b) & 0xFF) << 8) |       \
((PXE_UINT32)(a) & 0xFF))

#define PXE_BUSTYPE_PXE         PXE_BUSTYPE('!', 'P', 'X', 'E')

#define PXE_BUSTYPE_PCI         PXE_BUSTYPE('P', 'C', 'I', 'R')
#define PXE_BUSTYPE_PC_CARD     PXE_BUSTYPE('P', 'C', 'C', 'R')
#define PXE_BUSTYPE_USB         PXE_BUSTYPE('U', 'S', 'B', 'R')
#define PXE_BUSTYPE_1394        PXE_BUSTYPE('1', '3', '9', '4')

#define PXE_SWAP_UINT16(n)          \
((((PXE_UINT16)(n) & 0x00FF) << 8) |    \
(((PXE_UINT16)(n) & 0xFF00) >> 8))

#define PXE_SWAP_UINT32(n)              \
((((PXE_UINT32)(n) & 0x000000FF) << 24) |   \
(((PXE_UINT32)(n) & 0x0000FF00) << 8) |     \
(((PXE_UINT32)(n) & 0x00FF0000) >> 8) |     \
(((PXE_UINT32)(n) & 0xFF000000) >> 24))

#if PXE_UINT64_SUPPORT != 0
#define PXE_SWAP_UINT64(n)                  \
((((PXE_UINT64)(n) & 0x00000000000000FF) << 56) |   \
(((PXE_UINT64)(n) & 0x000000000000FF00) << 40) |    \
(((PXE_UINT64)(n) & 0x0000000000FF0000) << 24) |    \
(((PXE_UINT64)(n) & 0x00000000FF000000) << 8) | \
(((PXE_UINT64)(n) & 0x000000FF00000000) >> 8) | \
(((PXE_UINT64)(n) & 0x0000FF0000000000) >> 24) |    \
(((PXE_UINT64)(n) & 0x00FF000000000000) >> 40) |    \
(((PXE_UINT64)(n) & 0xFF00000000000000) >> 56))
#endif

#if PXE_NO_UINT64_SUPPORT != 0
#define PXE_SWAP_UINT64(n)                      \
{                                       \
PXE_UINT32 tmp = (PXE_UINT64)(n)[1];                \
(PXE_UINT64)(n)[1] = PXE_SWAP_UINT32((PXE_UINT64)(n)[0]);   \
(PXE_UINT64)(n)[0] = tmp;                       \
}
#endif

#define PXE_CPBSIZE_NOT_USED            0
#define PXE_DBSIZE_NOT_USED         0
#define PXE_CPBADDR_NOT_USED        (PXE_UINT64)0
#define PXE_DBADDR_NOT_USED     (PXE_UINT64)0

#define PXE_CONST const

#define PXE_VOLATILE volatile

typedef void PXE_VOID;

typedef unsigned char PXE_UINT8;

typedef unsigned short PXE_UINT16;

typedef unsigned PXE_UINT32;

#if PXE_UINT64_SUPPORT != 0

typedef UINT64 PXE_UINT64;
#endif

#if PXE_NO_UINT64_SUPPORT != 0
typedef PXE_UINT32 PXE_UINT64[2];
#endif

typedef unsigned PXE_UINTN;

typedef PXE_UINT8 PXE_BOOL;

#define PXE_FALSE               0
#define PXE_TRUE                    (!PXE_FALSE)

typedef PXE_UINT16 PXE_OPCODE;

#define PXE_OPCODE_GET_STATE                    0x0000

#define PXE_OPCODE_START                    0x0001

#define PXE_OPCODE_STOP                     0x0002

#define PXE_OPCODE_GET_INIT_INFO                0x0003

#define PXE_OPCODE_GET_CONFIG_INFO              0x0004

#define PXE_OPCODE_INITIALIZE                   0x0005

#define PXE_OPCODE_RESET                    0x0006

#define PXE_OPCODE_SHUTDOWN                 0x0007

#define PXE_OPCODE_INTERRUPT_ENABLES                0x0008

#define PXE_OPCODE_RECEIVE_FILTERS              0x0009

#define PXE_OPCODE_STATION_ADDRESS              0x000A

#define PXE_OPCODE_STATISTICS                   0x000B

#define PXE_OPCODE_MCAST_IP_TO_MAC              0x000C

#define PXE_OPCODE_NVDATA                   0x000D

#define PXE_OPCODE_GET_STATUS                   0x000E

#define PXE_OPCODE_FILL_HEADER              0x000F

#define PXE_OPCODE_TRANSMIT                 0x0010

#define PXE_OPCODE_RECEIVE                  0x0011

#define PXE_OPCODE_VALID_MAX                    0x0011

#define PXE_OPCODE_LAST_VALID                   0x0011

typedef PXE_UINT16 PXE_OPFLAGS;

#define PXE_OPFLAGS_NOT_USED                    0x0000

#define PXE_OPFLAGS_INITIALIZE_CABLE_DETECT_MASK    0x0001
#define PXE_OPFLAGS_INITIALIZE_DETECT_CABLE         0x0000
#define PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE  0x0001

#define PXE_OPFLAGS_RESET_DISABLE_INTERRUPTS        0x0001
#define PXE_OPFLAGS_RESET_DISABLE_FILTERS           0x0002

#define PXE_OPFLAGS_INTERRUPT_OPMASK                0xC000
#define PXE_OPFLAGS_INTERRUPT_ENABLE                0x8000
#define PXE_OPFLAGS_INTERRUPT_DISABLE           0x4000
#define PXE_OPFLAGS_INTERRUPT_READ              0x0000

#define PXE_OPFLAGS_INTERRUPT_RECEIVE           0x0001

#define PXE_OPFLAGS_INTERRUPT_TRANSMIT          0x0002

#define PXE_OPFLAGS_INTERRUPT_COMMAND           0x0004

#define PXE_OPFLAGS_INTERRUPT_SOFTWARE          0x0008

#define PXE_OPFLAGS_RECEIVE_FILTER_OPMASK           0xC000
#define PXE_OPFLAGS_RECEIVE_FILTER_ENABLE           0x8000
#define PXE_OPFLAGS_RECEIVE_FILTER_DISABLE          0x4000
#define PXE_OPFLAGS_RECEIVE_FILTER_READ         0x0000

#define PXE_OPFLAGS_RECEIVE_FILTER_RESET_MCAST_LIST 0x2000

#define PXE_OPFLAGS_RECEIVE_FILTER_UNICAST          0x0001

#define PXE_OPFLAGS_RECEIVE_FILTER_BROADCAST        0x0002

#define PXE_OPFLAGS_RECEIVE_FILTER_FILTERED_MULTICAST   0x0004

#define PXE_OPFLAGS_RECEIVE_FILTER_PROMISCUOUS      0x0008

#define PXE_OPFLAGS_RECEIVE_FILTER_ALL_MULTICAST        0x0010

#define PXE_OPFLAGS_STATION_ADDRESS_READ            0x0000
#define PXE_OPFLAGS_STATION_ADDRESS_RESET           0x0001

#define PXE_OPFLAGS_STATISTICS_READ             0x0000
#define PXE_OPFLAGS_STATISTICS_RESET                0x0001

#define PXE_OPFLAGS_MCAST_IP_TO_MAC_OPMASK          0x0003
#define PXE_OPFLAGS_MCAST_IPV4_TO_MAC           0x0000
#define PXE_OPFLAGS_MCAST_IPV6_TO_MAC           0x0001

#define PXE_OPFLAGS_NVDATA_OPMASK               0x0001
#define PXE_OPFLAGS_NVDATA_READ             0x0000
#define PXE_OPFLAGS_NVDATA_WRITE                0x0001

#define PXE_OPFLAGS_GET_INTERRUPT_STATUS            0x0001

#define PXE_OPFLAGS_GET_TRANSMITTED_BUFFERS         0x0002

#define PXE_OPFLAGS_FILL_HEADER_OPMASK          0x0001
#define PXE_OPFLAGS_FILL_HEADER_FRAGMENTED          0x0001
#define PXE_OPFLAGS_FILL_HEADER_WHOLE           0x0000

#define PXE_OPFLAGS_SWUNDI_TRANSMIT_OPMASK          0x0001
#define PXE_OPFLAGS_TRANSMIT_BLOCK              0x0001
#define PXE_OPFLAGS_TRANSMIT_DONT_BLOCK         0x0000

#define PXE_OPFLAGS_TRANSMIT_OPMASK             0x0002
#define PXE_OPFLAGS_TRANSMIT_FRAGMENTED         0x0002
#define PXE_OPFLAGS_TRANSMIT_WHOLE              0x0000

typedef PXE_UINT16 PXE_STATFLAGS;

#define PXE_STATFLAGS_INITIALIZE                0x0000

#define PXE_STATFLAGS_STATUS_MASK               0xC000
#define PXE_STATFLAGS_COMMAND_COMPLETE          0xC000
#define PXE_STATFLAGS_COMMAND_FAILED                0x8000
#define PXE_STATFLAGS_COMMAND_QUEUED                0x4000

#define PXE_STATFLAGS_DB_WRITE_TRUNCATED            0x2000

#define PXE_STATFLAGS_GET_STATE_MASK                0x0003
#define PXE_STATFLAGS_GET_STATE_INITIALIZED         0x0002
#define PXE_STATFLAGS_GET_STATE_STARTED         0x0001
#define PXE_STATFLAGS_GET_STATE_STOPPED         0x0000

#define PXE_STATFLAGS_CABLE_DETECT_MASK          0x0001
#define PXE_STATFLAGS_CABLE_DETECT_NOT_SUPPORTED 0x0000
#define PXE_STATFLAGS_CABLE_DETECT_SUPPORTED     0x0001

#define PXE_STATFLAGS_INITIALIZED_NO_MEDIA          0x0001

#define PXE_STATFLAGS_RESET_NO_MEDIA                0x0001

#define PXE_STATFLAGS_INTERRUPT_RECEIVE         0x0001

#define PXE_STATFLAGS_INTERRUPT_TRANSMIT            0x0002

#define PXE_STATFLAGS_INTERRUPT_COMMAND         0x0004

#define PXE_STATFLAGS_RECEIVE_FILTER_UNICAST        0x0001

#define PXE_STATFLAGS_RECEIVE_FILTER_BROADCAST      0x0002

#define PXE_STATFLAGS_RECEIVE_FILTER_FILTERED_MULTICAST 0x0004

#define PXE_STATFLAGS_RECEIVE_FILTER_PROMISCUOUS        0x0008

#define PXE_STATFLAGS_RECEIVE_FILTER_ALL_MULTICAST  0x0010

#define PXE_STATFLAGS_GET_STATUS_INTERRUPT_MASK     0x000F
#define PXE_STATFLAGS_GET_STATUS_NO_INTERRUPTS      0x0000

#define PXE_STATFLAGS_GET_STATUS_RECEIVE            0x0001

#define PXE_STATFLAGS_GET_STATUS_TRANSMIT           0x0002

#define PXE_STATFLAGS_GET_STATUS_COMMAND            0x0004

#define PXE_STATFLAGS_GET_STATUS_SOFTWARE           0x0008

#define PXE_STATFLAGS_GET_STATUS_TXBUF_QUEUE_EMPTY  0x0010

#define PXE_STATFLAGS_GET_STATUS_NO_TXBUFS_WRITTEN  0x0020

typedef PXE_UINT16 PXE_STATCODE;

#define PXE_STATCODE_INITIALIZE             0x0000

#define PXE_STATCODE_SUCCESS                    0x0000

#define PXE_STATCODE_INVALID_CDB                0x0001
#define PXE_STATCODE_INVALID_CPB                0x0002
#define PXE_STATCODE_BUSY                   	0x0003
#define PXE_STATCODE_QUEUE_FULL             	0x0004
#define PXE_STATCODE_ALREADY_STARTED            0x0005
#define PXE_STATCODE_NOT_STARTED                0x0006
#define PXE_STATCODE_NOT_SHUTDOWN               0x0007
#define PXE_STATCODE_ALREADY_INITIALIZED        0x0008
#define PXE_STATCODE_NOT_INITIALIZED            0x0009
#define PXE_STATCODE_DEVICE_FAILURE             0x000A
#define PXE_STATCODE_NVDATA_FAILURE             0x000B
#define PXE_STATCODE_UNSUPPORTED                0x000C
#define PXE_STATCODE_BUFFER_FULL                0x000D
#define PXE_STATCODE_INVALID_PARAMETER		0x000E
#define PXE_STATCODE_INVALID_UNDI		0x000F
#define PXE_STATCODE_IPV4_NOT_SUPPORTED		0x0010
#define PXE_STATCODE_IPV6_NOT_SUPPORTED		0x0011
#define PXE_STATCODE_NOT_ENOUGH_MEMORY		0x0012
#define PXE_STATCODE_NO_DATA			0x0013

typedef PXE_UINT16 PXE_IFNUM;

#define PXE_IFNUM_START                     0x0000

#define PXE_IFNUM_INVALID                   0x0000

typedef PXE_UINT16 PXE_CONTROL;

#define PXE_CONTROL_QUEUE_IF_BUSY               0x0002

#define PXE_CONTROL_LINK                    0x0001
#define PXE_CONTROL_LAST_CDB_IN_LIST                0x0000

typedef PXE_UINT8 PXE_FRAME_TYPE;

#define PXE_FRAME_TYPE_NONE                 0x00
#define PXE_FRAME_TYPE_UNICAST              0x01
#define PXE_FRAME_TYPE_BROADCAST                0x02
#define PXE_FRAME_TYPE_MULTICAST            0x03
#define PXE_FRAME_TYPE_PROMISCUOUS              0x04

typedef PXE_UINT32 PXE_IPV4;

typedef PXE_UINT32 PXE_IPV6[4];
#define PXE_MAC_LENGTH 32

typedef PXE_UINT8 PXE_MAC_ADDR[PXE_MAC_LENGTH];

typedef PXE_UINT8 PXE_IFTYPE;
typedef PXE_UINT16 PXE_MEDIA_PROTOCOL;

#define PXE_IFTYPE_ETHERNET                 0x01
#define PXE_IFTYPE_TOKENRING                    0x04
#define PXE_IFTYPE_FIBRE_CHANNEL                0x12

typedef struct s_pxe_hw_undi {
PXE_UINT32 Signature;
PXE_UINT8 Len;
PXE_UINT8 Fudge;
PXE_UINT8 Rev;
PXE_UINT8 IFcnt;
PXE_UINT8 MajorVer;
PXE_UINT8 MinorVer;
PXE_UINT16 reserved;
PXE_UINT32 Implementation;

} PXE_HW_UNDI;

#define PXE_HWSTAT_STATE_MASK                   0xC0000000
#define PXE_HWSTAT_BUSY                     0xC0000000
#define PXE_HWSTAT_INITIALIZED              0x80000000
#define PXE_HWSTAT_STARTED                  0x40000000
#define PXE_HWSTAT_STOPPED                  0x00000000

#define PXE_HWSTAT_COMMAND_FAILED               0x20000000

#define PXE_HWSTAT_PROMISCUOUS_MULTICAST_RX_ENABLED 0x00001000
#define PXE_HWSTAT_PROMISCUOUS_RX_ENABLED           0x00000800
#define PXE_HWSTAT_BROADCAST_RX_ENABLED         0x00000400
#define PXE_HWSTAT_MULTICAST_RX_ENABLED         0x00000200
#define PXE_HWSTAT_UNICAST_RX_ENABLED           0x00000100

#define PXE_HWSTAT_SOFTWARE_INT_ENABLED         0x00000080
#define PXE_HWSTAT_TX_COMPLETE_INT_ENABLED          0x00000040
#define PXE_HWSTAT_PACKET_RX_INT_ENABLED            0x00000020
#define PXE_HWSTAT_CMD_COMPLETE_INT_ENABLED         0x00000010

#define PXE_HWSTAT_SOFTWARE_INT_PENDING         0x00000008
#define PXE_HWSTAT_TX_COMPLETE_INT_PENDING          0x00000004
#define PXE_HWSTAT_PACKET_RX_INT_PENDING            0x00000002
#define PXE_HWSTAT_CMD_COMPLETE_INT_PENDING         0x00000001

#define PXE_HWCMD_ISSUE_COMMAND             0x80000000
#define PXE_HWCMD_INTS_AND_FILTS                0x00000000

#define PXE_HWCMD_PROMISCUOUS_MULTICAST_RX_ENABLE       0x00001000
#define PXE_HWCMD_PROMISCUOUS_RX_ENABLE         0x00000800
#define PXE_HWCMD_BROADCAST_RX_ENABLE           0x00000400
#define PXE_HWCMD_MULTICAST_RX_ENABLE           0x00000200
#define PXE_HWCMD_UNICAST_RX_ENABLE             0x00000100

#define PXE_HWCMD_SOFTWARE_INT_ENABLE           0x00000080
#define PXE_HWCMD_TX_COMPLETE_INT_ENABLE            0x00000040
#define PXE_HWCMD_PACKET_RX_INT_ENABLE          0x00000020
#define PXE_HWCMD_CMD_COMPLETE_INT_ENABLE           0x00000010

#define PXE_HWCMD_CLEAR_SOFTWARE_INT                0x00000008
#define PXE_HWCMD_CLEAR_TX_COMPLETE_INT         0x00000004
#define PXE_HWCMD_CLEAR_PACKET_RX_INT           0x00000002
#define PXE_HWCMD_CLEAR_CMD_COMPLETE_INT            0x00000001

typedef struct s_pxe_sw_undi {
PXE_UINT32 Signature;
PXE_UINT8 Len;
PXE_UINT8 Fudge;
PXE_UINT8 Rev;
PXE_UINT8 IFcnt;
PXE_UINT8 MajorVer;
PXE_UINT8 MinorVer;
PXE_UINT16 reserved1;
PXE_UINT32 Implementation;
PXE_UINT64 EntryPoint;
PXE_UINT8 reserved2[3];
PXE_UINT8 BusCnt;
PXE_UINT32 BusType[1];
} PXE_SW_UNDI;

typedef union u_pxe_undi {
PXE_HW_UNDI hw;
PXE_SW_UNDI sw;
} PXE_UNDI;

#define PXE_ROMID_SIGNATURE     PXE_BUSTYPE('!', 'P', 'X', 'E')

#define PXE_ROMID_REV                       0x02

#define PXE_ROMID_MAJORVER                  0x03
#define PXE_ROMID_MINORVER                  0x00

#define PXE_ROMID_IMP_HW_UNDI                   0x80000000
#define PXE_ROMID_IMP_SW_VIRT_ADDR              0x40000000
#define PXE_ROMID_IMP_64BIT_DEVICE              0x00010000
#define PXE_ROMID_IMP_FRAG_SUPPORTED                0x00008000
#define PXE_ROMID_IMP_CMD_LINK_SUPPORTED            0x00004000
#define PXE_ROMID_IMP_CMD_QUEUE_SUPPORTED           0x00002000
#define PXE_ROMID_IMP_MULTI_FRAME_SUPPORTED         0x00001000
#define PXE_ROMID_IMP_NVDATA_SUPPORT_MASK           0x00000C00
#define PXE_ROMID_IMP_NVDATA_BULK_WRITABLE          0x00000C00
#define PXE_ROMID_IMP_NVDATA_SPARSE_WRITABLE        0x00000800
#define PXE_ROMID_IMP_NVDATA_READ_ONLY          0x00000400
#define PXE_ROMID_IMP_NVDATA_NOT_AVAILABLE          0x00000000
#define PXE_ROMID_IMP_STATISTICS_SUPPORTED          0x00000200
#define PXE_ROMID_IMP_STATION_ADDR_SETTABLE         0x00000100
#define PXE_ROMID_IMP_PROMISCUOUS_MULTICAST_RX_SUPPORTED    0x00000080
#define PXE_ROMID_IMP_PROMISCUOUS_RX_SUPPORTED      0x00000040
#define PXE_ROMID_IMP_BROADCAST_RX_SUPPORTED        0x00000020
#define PXE_ROMID_IMP_FILTERED_MULTICAST_RX_SUPPORTED   0x00000010
#define PXE_ROMID_IMP_SOFTWARE_INT_SUPPORTED        0x00000008
#define PXE_ROMID_IMP_TX_COMPLETE_INT_SUPPORTED     0x00000004
#define PXE_ROMID_IMP_PACKET_RX_INT_SUPPORTED       0x00000002
#define PXE_ROMID_IMP_CMD_COMPLETE_INT_SUPPORTED        0x00000001

typedef struct s_pxe_cdb {
PXE_OPCODE OpCode;
PXE_OPFLAGS OpFlags;
PXE_UINT16 CPBsize;
PXE_UINT16 DBsize;
UINT64 CPBaddr;
UINT64 DBaddr;
PXE_STATCODE StatCode;
PXE_STATFLAGS StatFlags;
PXE_UINT16 IFnum;
PXE_CONTROL Control;
} PXE_CDB;

typedef union u_pxe_ip_addr {
PXE_IPV6 IPv6;
PXE_IPV4 IPv4;
} PXE_IP_ADDR;

typedef union pxe_device {

struct {

PXE_UINT32 BusType;

PXE_UINT16 Bus;
PXE_UINT8 Device;
PXE_UINT8 Function;
} PCI, PCC;

struct {
PXE_UINT32 BusType;
PXE_UINT32 tdb;
} USB, _1394;
} PXE_DEVICE;

#define MAX_PCI_CONFIG_LEN 64
#define MAX_EEPROM_LEN 128
#define MAX_XMIT_BUFFERS    32
#define MAX_MCAST_ADDRESS_CNT 8

typedef struct s_pxe_cpb_start {

    PXE_UINT64 Delay;

    PXE_UINT64 Block;

    PXE_UINT64 Virt2Phys;

    PXE_UINT64 Mem_IO;
} PXE_CPB_START;

#define PXE_DELAY_MILLISECOND                   1000
#define PXE_DELAY_SECOND                    1000000
#define PXE_IO_READ                     0
#define PXE_IO_WRITE                        1
#define PXE_MEM_READ                        2
#define PXE_MEM_WRITE                       4

typedef struct s_pxe_db_get_init_info {

    PXE_UINT32 MemoryRequired;

    PXE_UINT32 FrameDataLen;

    PXE_UINT32 LinkSpeeds[4];

    PXE_UINT32 NvCount;

    PXE_UINT16 NvWidth;

    PXE_UINT16 MediaHeaderLen;

    PXE_UINT16 HWaddrLen;

    PXE_UINT16 MCastFilterCnt;

    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;

    PXE_UINT8 IFtype;

    PXE_UINT8 Duplex;

    PXE_UINT8 LoopBack;
} PXE_DB_GET_INIT_INFO;

#define PXE_MAX_TXRX_UNIT_ETHER             1500

#define PXE_HWADDR_LEN_ETHER                    0x0006
#define PXE_MAC_HEADER_LEN_ETHER                0x000E

#define PXE_DUPLEX_ENABLE_FULL_SUPPORTED            1
#define PXE_DUPLEX_FORCE_FULL_SUPPORTED         2

#define PXE_LOOPBACK_INTERNAL_SUPPORTED         1
#define PXE_LOOPBACK_EXTERNAL_SUPPORTED         2

typedef struct s_pxe_pci_config_info {

    PXE_UINT32 BusType;

    PXE_UINT16 Bus;
    PXE_UINT8 Device;
    PXE_UINT8 Function;

    union {
        PXE_UINT8 Byte[256];
        PXE_UINT16 Word[128];
        PXE_UINT32 Dword[64];
    } Config;
} PXE_PCI_CONFIG_INFO;

typedef struct s_pxe_pcc_config_info {

    PXE_UINT32 BusType;

    PXE_UINT16 Bus;
    PXE_UINT8 Device;
    PXE_UINT8 Function;

    union {
        PXE_UINT8 Byte[256];
        PXE_UINT16 Word[128];
        PXE_UINT32 Dword[64];
    } Config;
} PXE_PCC_CONFIG_INFO;

typedef struct s_pxe_usb_config_info {
    PXE_UINT32 BusType;

} PXE_USB_CONFIG_INFO;

typedef struct s_pxe_1394_config_info {
    PXE_UINT32 BusType;

} PXE_1394_CONFIG_INFO;

typedef union u_pxe_db_get_config_info {
    PXE_PCI_CONFIG_INFO pci;
    PXE_PCC_CONFIG_INFO pcc;
    PXE_USB_CONFIG_INFO usb;
    PXE_1394_CONFIG_INFO _1394;
} PXE_DB_GET_CONFIG_INFO;

typedef struct s_pxe_cpb_initialize {

    PXE_UINT64 MemoryAddr;

    PXE_UINT32 MemoryLength;

    PXE_UINT32 LinkSpeed;

    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;

    PXE_UINT8 Duplex;

    PXE_UINT8 LoopBack;
} PXE_CPB_INITIALIZE;

#define PXE_DUPLEX_DEFAULT                  0x00
#define PXE_FORCE_FULL_DUPLEX                   0x01
#define PXE_ENABLE_FULL_DUPLEX              0x02

#define LOOPBACK_NORMAL 0
#define LOOPBACK_INTERNAL 1
#define LOOPBACK_EXTERNAL 2

typedef struct s_pxe_db_initialize {

    PXE_UINT32 MemoryUsed;

    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;
} PXE_DB_INITIALIZE;

typedef struct s_pxe_cpb_receive_filters {

    PXE_MAC_ADDR MCastList[MAX_MCAST_ADDRESS_CNT];
} PXE_CPB_RECEIVE_FILTERS;

typedef struct s_pxe_db_receive_filters {

    PXE_MAC_ADDR MCastList[MAX_MCAST_ADDRESS_CNT];
} PXE_DB_RECEIVE_FILTERS;

typedef struct s_pxe_cpb_station_address {

    PXE_MAC_ADDR StationAddr;
} PXE_CPB_STATION_ADDRESS;

typedef struct s_pxe_dpb_station_address {

    PXE_MAC_ADDR StationAddr;

    PXE_MAC_ADDR BroadcastAddr;

    PXE_MAC_ADDR PermanentAddr;
} PXE_DB_STATION_ADDRESS;

typedef struct s_pxe_db_statistics {

    PXE_UINT64 Supported;

    PXE_UINT64 Data[64];
} PXE_DB_STATISTICS;

#define PXE_STATISTICS_RX_TOTAL_FRAMES          0x00

#define PXE_STATISTICS_RX_GOOD_FRAMES           0x01

#define PXE_STATISTICS_RX_UNDERSIZE_FRAMES          0x02

#define PXE_STATISTICS_RX_OVERSIZE_FRAMES           0x03

#define PXE_STATISTICS_RX_DROPPED_FRAMES            0x04

#define PXE_STATISTICS_RX_UNICAST_FRAMES            0x05

#define PXE_STATISTICS_RX_BROADCAST_FRAMES          0x06

#define PXE_STATISTICS_RX_MULTICAST_FRAMES          0x07

#define PXE_STATISTICS_RX_CRC_ERROR_FRAMES          0x08

#define PXE_STATISTICS_RX_TOTAL_BYTES           0x09

#define PXE_STATISTICS_TX_TOTAL_FRAMES          0x0A
#define PXE_STATISTICS_TX_GOOD_FRAMES           0x0B
#define PXE_STATISTICS_TX_UNDERSIZE_FRAMES          0x0C
#define PXE_STATISTICS_TX_OVERSIZE_FRAMES           0x0D
#define PXE_STATISTICS_TX_DROPPED_FRAMES            0x0E
#define PXE_STATISTICS_TX_UNICAST_FRAMES            0x0F
#define PXE_STATISTICS_TX_BROADCAST_FRAMES          0x10
#define PXE_STATISTICS_TX_MULTICAST_FRAMES          0x11
#define PXE_STATISTICS_TX_CRC_ERROR_FRAMES          0x12
#define PXE_STATISTICS_TX_TOTAL_BYTES           0x13

#define PXE_STATISTICS_COLLISIONS               0x14

#define PXE_STATISTICS_UNSUPPORTED_PROTOCOL         0x15

typedef struct s_pxe_cpb_mcast_ip_to_mac {

    PXE_IP_ADDR IP;
} PXE_CPB_MCAST_IP_TO_MAC;

typedef struct s_pxe_db_mcast_ip_to_mac {

    PXE_MAC_ADDR MAC;
} PXE_DB_MCAST_IP_TO_MAC;

typedef struct s_pxe_cpb_nvdata_sparse {

    struct {

        PXE_UINT32 Addr;

        union {
            PXE_UINT8 Byte;
            PXE_UINT16 Word;
            PXE_UINT32 Dword;
        } Data;
    } Item[MAX_EEPROM_LEN];
} PXE_CPB_NVDATA_SPARSE;

typedef union u_pxe_cpb_nvdata_bulk {

    PXE_UINT8 Byte[MAX_EEPROM_LEN << 2];

    PXE_UINT16 Word[MAX_EEPROM_LEN << 1];

    PXE_UINT32 Dword[MAX_EEPROM_LEN];
} PXE_CPB_NVDATA_BULK;

typedef struct s_pxe_db_nvdata {

    union {

        PXE_UINT8 Byte[MAX_EEPROM_LEN << 2];

        PXE_UINT16 Word[MAX_EEPROM_LEN << 1];

        PXE_UINT32 Dword[MAX_EEPROM_LEN];
    } Data;
} PXE_DB_NVDATA;

typedef struct s_pxe_db_get_status {

    PXE_UINT32 RxFrameLen;

    PXE_UINT32 reserved;

    PXE_UINT64 TxBuffer[MAX_XMIT_BUFFERS];
} PXE_DB_GET_STATUS;

typedef struct s_pxe_cpb_fill_header {

    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

    PXE_UINT64 MediaHeader;

    PXE_UINT32 PacketLen;

    PXE_UINT16 Protocol;

    PXE_UINT16 MediaHeaderLen;
} PXE_CPB_FILL_HEADER;

#define PXE_PROTOCOL_ETHERNET_IP                0x0800
#define PXE_PROTOCOL_ETHERNET_ARP               0x0806
#define MAX_XMIT_FRAGMENTS 16

typedef struct s_pxe_cpb_fill_header_fragmented {

    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

    PXE_UINT32 PacketLen;

    PXE_MEDIA_PROTOCOL Protocol;

    PXE_UINT16 MediaHeaderLen;

    PXE_UINT16 FragCnt;

    PXE_UINT16 reserved;

    struct {

        PXE_UINT64 FragAddr;

        PXE_UINT32 FragLen;

        PXE_UINT32 reserved;
    } FragDesc[MAX_XMIT_FRAGMENTS];
} PXE_CPB_FILL_HEADER_FRAGMENTED;

typedef struct s_pxe_cpb_transmit {

    PXE_UINT64 FrameAddr;

    PXE_UINT32 DataLen;

    PXE_UINT16 MediaheaderLen;

    PXE_UINT16 reserved;
} PXE_CPB_TRANSMIT;

typedef struct s_pxe_cpb_transmit_fragments {

    PXE_UINT32 FrameLen;

    PXE_UINT16 MediaheaderLen;

    PXE_UINT16 FragCnt;

    struct {

        PXE_UINT64 FragAddr;

        PXE_UINT32 FragLen;

        PXE_UINT32 reserved;
    } FragDesc[MAX_XMIT_FRAGMENTS];
} PXE_CPB_TRANSMIT_FRAGMENTS;

typedef struct s_pxe_cpb_receive {

    PXE_UINT64 BufferAddr;

    PXE_UINT32 BufferLen;

    PXE_UINT32 reserved;
} PXE_CPB_RECEIVE;

typedef struct s_pxe_db_receive {

    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

    PXE_UINT32 FrameLen;

    PXE_MEDIA_PROTOCOL Protocol;

    PXE_UINT16 MediaHeaderLen;

    PXE_FRAME_TYPE Type;

    PXE_UINT8 reserved[7];

} PXE_DB_RECEIVE;

#pragma pack()

#endif
