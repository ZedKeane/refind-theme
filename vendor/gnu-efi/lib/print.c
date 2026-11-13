#include "lib.h"
#include "efistdarg.h"

#ifdef RUNTIME_CODE
#ifndef __GNUC__
#pragma RUNTIME_CODE(DbgPrint)

#endif
#endif

#define PRINT_STRING_LEN            200
#define PRINT_ITEM_BUFFER_LEN       100

typedef struct {
    BOOLEAN             Ascii;
    UINTN               Index;
    union {
        CONST CHAR16    *pw;
        CONST CHAR8     *pc;
    } un;
} POINTER;

#define pw	un.pw
#define pc	un.pc

typedef struct _pitem {

    POINTER     Item;
    CHAR16      Scratch[PRINT_ITEM_BUFFER_LEN];
    UINTN       Width;
    UINTN       FieldWidth;
    UINTN       *WidthParse;
    CHAR16      Pad;
    BOOLEAN     PadBefore;
    BOOLEAN     Comma;
    BOOLEAN     Long;
} PRINT_ITEM;

typedef struct _pstate {

    POINTER     fmt;
    va_list     args;

    CHAR16      *Buffer;
    CHAR16      *End;
    CHAR16      *Pos;
    UINTN       Len;

    UINTN       Attr;
    UINTN       RestoreAttr;

    UINTN       AttrNorm;
    UINTN       AttrHighlight;
    UINTN       AttrError;

    INTN        (EFIAPI *Output)(VOID *context, CHAR16 *str);
    INTN        (EFIAPI *SetAttr)(VOID *context, UINTN attr);
    VOID        *Context;

    struct _pitem  *Item;
} PRINT_STATE;

STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    );

STATIC
UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CONST CHAR16                     *fmt,
    IN CONST CHAR8                      *fmta,
    IN va_list                          args
    );

STATIC
INTN EFIAPI
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    );

STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    );

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    );

STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    );

STATIC
CHAR16
PGETC (
    IN POINTER      *p
    );

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    );

INTN EFIAPI
_SPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    );

INTN EFIAPI
_PoolPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    );

INTN
DbgPrint (
    IN INTN         mask,
    IN CONST CHAR8  *fmt,
    ...
    )

{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;
    PRINT_STATE     ps;
    va_list         args;
    UINTN           back;
    UINTN           attr;
    UINTN           SavedAttribute;

    if (!(EFIDebug & mask)) {
        return 0;
    }

    va_start (args, fmt);
    ZeroMem (&ps, sizeof(ps));

    ps.Output = _DbgOut;
    ps.fmt.Ascii = TRUE;
    ps.fmt.pc = fmt;
    va_copy(ps.args, args);
    ps.Attr = EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED);

    DbgOut = LibRuntimeDebugOut;

    if (!DbgOut) {
        DbgOut = ST->StdErr;
    }

    if (DbgOut) {
        ps.Attr = DbgOut->Mode->Attribute;
        ps.Context = DbgOut;
        ps.SetAttr = (INTN (EFIAPI *)(VOID *, UINTN))  DbgOut->SetAttribute;
    }

    SavedAttribute = ps.Attr;

    back = (ps.Attr >> 4) & 0xf;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    attr = ps.AttrNorm;

    if (mask & D_WARN) {
        attr = ps.AttrHighlight;
    }

    if (mask & D_ERROR) {
        attr = ps.AttrError;
    }

    if (ps.SetAttr) {
        ps.Attr = attr;
        uefi_call_wrapper(ps.SetAttr, 2, ps.Context, attr);
    }

    _Print (&ps);

    va_end (ps.args);
    va_end (args);

    if (ps.SetAttr) {
        uefi_call_wrapper(ps.SetAttr, 2, ps.Context, SavedAttribute);
    }

    return 0;
}

STATIC
INTN
IsLocalPrint(void *func)
{
	if (func == _DbgOut || func == _SPrint || func == _PoolPrint)
		return 1;
	return 0;
}

STATIC
INTN EFIAPI
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )

{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;

    DbgOut = Context;

    if (DbgOut) {
	if (IsLocalPrint(DbgOut->OutputString))
		DbgOut->OutputString(DbgOut, Buffer);
        else
		uefi_call_wrapper(DbgOut->OutputString, 2, DbgOut, Buffer);
    }

    return 0;
}

INTN EFIAPI
_SPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )

{
    UINTN           len;
    POOL_PRINT      *spc;

    spc = Context;
    len = StrLen(Buffer);

    if (spc->len + len > spc->maxlen) {
        len = spc->maxlen - spc->len;
    }

    CopyMem (spc->str + spc->len, Buffer, len * sizeof(CHAR16));
    spc->len += len;

    if (spc->len < spc->maxlen) {
        spc->str[spc->len] = 0;
    } else if (spc->maxlen) {
        spc->str[spc->maxlen] = 0;
    }

    return 0;
}

INTN EFIAPI
_PoolPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )

{
    UINTN           newlen;
    POOL_PRINT      *spc;

    spc = Context;
    newlen = spc->len + StrLen(Buffer) + 1;

    if (newlen > spc->maxlen) {

        newlen += PRINT_STRING_LEN;
        spc->maxlen = newlen;
        spc->str = ReallocatePool (
                        spc->str,
                        spc->len * sizeof(CHAR16),
                        spc->maxlen * sizeof(CHAR16)
                        );

        if (!spc->str) {
            spc->len = 0;
            spc->maxlen = 0;
        }
    }

    return _SPrint (Context, Buffer);
}

VOID
_PoolCatPrint (
    IN CONST CHAR16     *fmt,
    IN va_list          args,
    IN OUT POOL_PRINT   *spc,
    IN INTN             (EFIAPI *Output)(VOID *context, CHAR16 *str)
    )

{
    PRINT_STATE         ps;

    ZeroMem (&ps, sizeof(ps));
    ps.Output  = Output;
    ps.Context = spc;
    ps.fmt.pw = fmt;
    va_copy(ps.args, args);
    _Print (&ps);
    va_end(ps.args);
}

UINTN
UnicodeVSPrint (
    OUT CHAR16        *Str,
    IN UINTN          StrSize,
    IN CONST CHAR16   *fmt,
    va_list           args
    )

{
    POOL_PRINT          spc;

    spc.str    = Str;
    spc.maxlen = StrSize / sizeof(CHAR16) - 1;
    spc.len    = 0;

    _PoolCatPrint (fmt, args, &spc, _SPrint);

    return spc.len;
}

UINTN
UnicodeSPrint (
    OUT CHAR16        *Str,
    IN UINTN          StrSize,
    IN CONST CHAR16   *fmt,
    ...
    )

{
    va_list          args;
    UINTN            len;

    va_start (args, fmt);
    len = UnicodeVSPrint(Str, StrSize, fmt, args);
    va_end (args);

    return len;
}

CHAR16 *
VPoolPrint (
    IN CONST CHAR16     *fmt,
    va_list             args
    )

{
    POOL_PRINT          spc;
    ZeroMem (&spc, sizeof(spc));
    _PoolCatPrint (fmt, args, &spc, _PoolPrint);
    return spc.str;
}

CHAR16 *
PoolPrint (
    IN CONST CHAR16     *fmt,
    ...
    )

{
    va_list args;
    CHAR16 *pool;
    va_start (args, fmt);
    pool = VPoolPrint(fmt, args);
    va_end (args);
    return pool;
}

CHAR16 *
CatPrint (
    IN OUT POOL_PRINT   *Str,
    IN CONST CHAR16     *fmt,
    ...
    )

{
    va_list             args;

    va_start (args, fmt);
    _PoolCatPrint (fmt, args, Str, _PoolPrint);
    va_end (args);
    return Str->str;
}

UINTN
Print (
    IN CONST CHAR16   *fmt,
    ...
    )

{
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, fmt, NULL, args);
    va_end (args);
    return back;
}

UINTN
VPrint (
    IN CONST CHAR16   *fmt,
    va_list           args
    )

{
    return _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, fmt, NULL, args);
}

UINTN
PrintAt (
    IN UINTN          Column,
    IN UINTN          Row,
    IN CONST CHAR16   *fmt,
    ...
    )

{
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = _IPrint (Column, Row, ST->ConOut, fmt, NULL, args);
    va_end (args);
    return back;
}

UINTN
IPrint (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out,
    IN CONST CHAR16                    *fmt,
    ...
    )

{
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = _IPrint ((UINTN) -1, (UINTN) -1, Out, fmt, NULL, args);
    va_end (args);
    return back;
}

UINTN
IPrintAt (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN CONST CHAR16                     *fmt,
    ...
    )

{
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = _IPrint (Column, Row, Out, fmt, NULL, args);
    va_end (args);
    return back;
}

UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CONST CHAR16                     *fmt,
    IN CONST CHAR8                      *fmta,
    IN va_list                          args
    )

{
    PRINT_STATE     ps;
    UINTN            back;

    ZeroMem (&ps, sizeof(ps));
    ps.Context = Out;
    ps.Output  = (INTN (EFIAPI *)(VOID *, CHAR16 *)) Out->OutputString;
    ps.SetAttr = (INTN (EFIAPI *)(VOID *, UINTN))  Out->SetAttribute;
    ps.Attr = Out->Mode->Attribute;

    back = (ps.Attr >> 4) & 0xF;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    if (fmt) {
        ps.fmt.pw = fmt;
    } else {
        ps.fmt.Ascii = TRUE;
        ps.fmt.pc = fmta;
    }

    va_copy(ps.args, args);

    if (Column != (UINTN) -1) {
        uefi_call_wrapper(Out->SetCursorPosition, 3, Out, Column, Row);
    }

    back = _Print (&ps);
    va_end(ps.args);
    return back;
}

UINTN
AsciiPrint (
    IN CONST CHAR8    *fmt,
    ...
    )

{
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, NULL, fmt, args);
    va_end (args);
    return back;
}

UINTN
AsciiVSPrint (
    OUT CHAR8         *Str,
    IN UINTN          StrSize,
    IN CONST CHAR8    *fmt,
    va_list           args
)

{
    CHAR16 *UnicodeStr, *UnicodeFmt;
    UINTN i, Len;

    UnicodeStr = AllocatePool(StrSize * sizeof(CHAR16));
    if (!UnicodeStr)
        return 0;

    UnicodeFmt = PoolPrint(L"%a", fmt);
    if (!UnicodeFmt) {
        FreePool(UnicodeStr);
        return 0;
    }

    Len = UnicodeVSPrint(UnicodeStr, StrSize, UnicodeFmt, args);
    FreePool(UnicodeFmt);

    for (i = 0; i < Len; i++)
        Str[i] = (CHAR8)UnicodeStr[i];
    Str[Len] = 0;
    FreePool(UnicodeStr);

    return Len;
}

STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    )
{
    *ps->Pos = 0;
    if (IsLocalPrint(ps->Output))
	ps->Output(ps->Context, ps->Buffer);
    else
    	uefi_call_wrapper(ps->Output, 2, ps->Context, ps->Buffer);
    ps->Pos = ps->Buffer;
}

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    )
{
   PFLUSH (ps);

   ps->RestoreAttr = ps->Attr;
   if (ps->SetAttr) {
	uefi_call_wrapper(ps->SetAttr, 2, ps->Context, Attr);
   }

   ps->Attr = Attr;
}

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    )
{

    if (c == '\n') {
        PPUTC (ps, '\r');
    }

    *ps->Pos = c;
    ps->Pos += 1;
    ps->Len += 1;

    if (ps->Pos >= ps->End) {
        PFLUSH(ps);
    }
}

STATIC
CHAR16
PGETC (
    IN POINTER      *p
    )
{
    CHAR16      c;

    c = p->Ascii ? p->pc[p->Index] : p->pw[p->Index];
    p->Index += 1;

    return  c;
}

STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    )
{
    UINTN               Len, i;
    PRINT_ITEM          *Item;
    CHAR16              c;

    Item = ps->Item;
    Item->Item.Index = 0;
    while (Item->Item.Index < Item->FieldWidth) {
        c = PGETC(&Item->Item);
        if (!c) {
            Item->Item.Index -= 1;
            break;
        }
    }
    Len = Item->Item.Index;

    if (Item->FieldWidth == (UINTN) -1) {
        Item->FieldWidth = Len;
    }

    if (Len > Item->Width) {
        Item->Width = Len;
    }

    if (Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }

    for (i=Len; i < Item->Width; i++) {
        PPUTC (ps, Item->Pad);
    }

    Item->Item.Index=0;
    while (Item->Item.Index < Len) {
        PPUTC (ps, PGETC(&Item->Item));
    }

    if (!Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }
}

STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    )

{
    CHAR16          c;
    UINTN           Attr;
    PRINT_ITEM      Item;
    CHAR16          Buffer[PRINT_STRING_LEN];

    ps->Len = 0;
    ps->Buffer = Buffer;
    ps->Pos = Buffer;
    ps->End = Buffer + PRINT_STRING_LEN - 1;
    ps->Item = &Item;

    ps->fmt.Index = 0;
    while ((c = PGETC(&ps->fmt))) {

        if (c != '%') {
            PPUTC ( ps, c );
            continue;
        }

        Item.FieldWidth = (UINTN) -1;
        Item.Width = 0;
        Item.WidthParse = &Item.Width;
        Item.Pad = ' ';
        Item.PadBefore = TRUE;
        Item.Comma = FALSE;
        Item.Long = FALSE;
        Item.Item.Ascii = FALSE;
        Item.Item.pw = NULL;
        ps->RestoreAttr = 0;
        Attr = 0;

        while ((c = PGETC(&ps->fmt))) {

            switch (c) {

            case '%':

                Item.Scratch[0] = '%';
                Item.Scratch[1] = 0;
                Item.Item.pw = Item.Scratch;
                break;

            case ',':
                Item.Comma = TRUE;
                break;

            case '-':
                Item.PadBefore = FALSE;
                break;

            case '*':
                *Item.WidthParse = va_arg(ps->args, UINTN);
                break;

            case '.':
                Item.WidthParse = &Item.FieldWidth;
                break;

            case '0':
                Item.Pad = '0';
                break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                *Item.WidthParse = 0;
                do {
                    *Item.WidthParse = *Item.WidthParse * 10 + c - '0';
                    c = PGETC(&ps->fmt);
                } while (c >= '0'  &&  c <= '9') ;
                ps->fmt.Index -= 1;
                break;

            case 'a':
                Item.Item.pc = va_arg(ps->args, CHAR8 *);
                Item.Item.Ascii = TRUE;
                if (!Item.Item.pc) {
                    Item.Item.pc = (CHAR8 *)"(null)";
                }
                break;

            case 'c':
                Item.Scratch[0] = (CHAR16) va_arg(ps->args, UINTN);
                Item.Scratch[1] = 0;
                Item.Item.pw = Item.Scratch;
                break;

            case 'D':
            {
                EFI_DEVICE_PATH *dp = va_arg(ps->args, EFI_DEVICE_PATH *);
                CHAR16 *dpstr = DevicePathToStr(dp);
                StrnCpy(Item.Scratch, dpstr, PRINT_ITEM_BUFFER_LEN);
                Item.Scratch[PRINT_ITEM_BUFFER_LEN-1] = L'\0';
                FreePool(dpstr);

                Item.Item.pw = Item.Scratch;
                break;
            }

            case 'd':
                ValueToString (
                    Item.Scratch,
                    Item.Comma,
                    Item.Long ? va_arg(ps->args, INT64) : va_arg(ps->args, INT32)
                    );
                Item.Item.pw = Item.Scratch;
                break;

            case 'E':
                Attr = ps->AttrError;
                break;

            case 'e':
                PSETATTR(ps, ps->AttrError);
                break;

            case 'f':
                FloatToString (
                    Item.Scratch,
                    Item.Comma,
                    va_arg(ps->args, double)
                    );
                Item.Item.pw = Item.Scratch;
                break;

            case 'g':
                GuidToString (Item.Scratch, va_arg(ps->args, EFI_GUID *));
                Item.Item.pw = Item.Scratch;
                break;

            case 'H':
                Attr = ps->AttrHighlight;
                break;

            case 'h':
                PSETATTR(ps, ps->AttrHighlight);
                break;

            case 'l':
                Item.Long = TRUE;
                break;

            case 'N':
                Attr = ps->AttrNorm;
                break;

            case 'n':
                PSETATTR(ps, ps->AttrNorm);
                break;

            case 'p':
                Item.Width = sizeof(void *) == (8 ? 16 : 8) + 2;
                Item.Pad = '0';
                Item.Scratch[0] = ' ';
                Item.Scratch[1] = ' ';
                ValueToHex (
                    Item.Scratch+2,
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                    );
                Item.Scratch[0] = '0';
                Item.Scratch[1] = 'x';
                Item.Item.pw = Item.Scratch;
                break;

            case 'r':
                StatusToString (Item.Scratch, va_arg(ps->args, EFI_STATUS));
                Item.Item.pw = Item.Scratch;
                break;

            case 's':
                Item.Item.pw = va_arg(ps->args, CHAR16 *);
                if (!Item.Item.pw) {
                    Item.Item.pw = L"(null)";
                }
                break;

            case 't':
                TimeToString (Item.Scratch, va_arg(ps->args, EFI_TIME *));
                Item.Item.pw = Item.Scratch;
                break;

            case 'u':
                ValueToString (
                    Item.Scratch,
                    Item.Comma,
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                    );
                Item.Item.pw = Item.Scratch;
                break;

            case 'X':
                Item.Width = Item.Long ? 16 : 8;
                Item.Pad = '0';
#if __GNUC__ >= 7
		__attribute__ ((fallthrough));
#endif
            case 'x':
                ValueToHex (
                    Item.Scratch,
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                    );
                Item.Item.pw = Item.Scratch;
                break;

            default:
                Item.Scratch[0] = '?';
                Item.Scratch[1] = 0;
                Item.Item.pw = Item.Scratch;
                break;
            }

            if (Item.Item.pw) {
                PITEM (ps);
                break;
            }

            if (Attr) {
                PSETATTR(ps, Attr);
                ps->RestoreAttr = 0;
                break;
            }
        }

        if (ps->RestoreAttr) {
            PSETATTR(ps, ps->RestoreAttr);
        }
    }

    PFLUSH (ps);
    return ps->Len;
}

STATIC CHAR8 Hex[] = {'0','1','2','3','4','5','6','7',
                      '8','9','A','B','C','D','E','F'};

VOID
ValueToHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
    )
{
    CHAR8           str[30], *p1;
    CHAR16          *p2;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    while (v) {

        *(p1++) = Hex[(UINTN)(v & 0xf)];
        v = RShiftU64 (v, 4);
    }

    while (p1 != str) {
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID
ValueToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN INT64    v
    )
{
    STATIC CHAR8 ca[] = {  3, 1, 2 };
    CHAR8        str[40], *p1;
    CHAR16       *p2;
    UINTN        c, r;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    if (v < 0) {
        *(p2++) = '-';
        v = -v;
    }

    while (v) {
        v = (INT64)DivU64x32 ((UINT64)v, 10, &r);
        *(p1++) = (CHAR8)r + '0';
    }

    c = (UINTN) (Comma ? ca[(p1 - str) % 3] : 999) + 1;
    while (p1 != str) {

        c -= 1;
        if (!c) {
            *(p2++) = ',';
            c = 3;
        }

        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID
FloatToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN double   v
    )
{

    INTN i = (INTN)v;
    ValueToString(Buffer, Comma, i);

    UINTN x = StrLen(Buffer);
    Buffer[x] = L'.';
    x++;

    float f = (float)(v - i);
    if (f < 0) f = -f;

    f *= 10.0;
    while (   (f != 0)
           && ((INTN)f == 0))
    {
      Buffer[x] = L'0';
      x++;
      f *= 10.0;
    }

    while ((float)(INTN)f != f)
    {
      f *= 10;
    }
    ValueToString(Buffer + x, FALSE, (INTN)f);
    return;
}

VOID
TimeToString (
    OUT CHAR16      *Buffer,
    IN EFI_TIME     *Time
    )
{
    UINTN       Hour, Year;
    CHAR16      AmPm;

    AmPm = 'a';
    Hour = Time->Hour;
    if (Time->Hour == 0) {
        Hour = 12;
    } else if (Time->Hour >= 12) {
        AmPm = 'p';
        if (Time->Hour >= 13) {
            Hour -= 12;
        }
    }

    Year = Time->Year % 100;

    UnicodeSPrint (Buffer, 0, L"%02d/%02d/%02d  %02d:%02d%c",
        Time->Month,
        Time->Day,
        Year,
        Hour,
        Time->Minute,
        AmPm
        );
}

VOID
DumpHex (
    IN UINTN        Indent,
    IN UINTN        Offset,
    IN UINTN        DataSize,
    IN VOID         *UserData
    )
{
    CHAR8           *Data, Val[50], Str[20], c;
    UINTN           Size, Index;

    UINTN           ScreenCount;
    UINTN           TempColumn;
    UINTN           ScreenSize;
    CHAR16          ReturnStr[1];

    uefi_call_wrapper(ST->ConOut->QueryMode, 4, ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
    ScreenCount = 0;
    ScreenSize -= 2;

    Data = UserData;
    while (DataSize) {
        Size = 16;
        if (Size > DataSize) {
            Size = DataSize;
        }

        for (Index=0; Index < Size; Index += 1) {
            c = Data[Index];
            Val[Index*3+0] = Hex[c>>4];
            Val[Index*3+1] = Hex[c&0xF];
            Val[Index*3+2] = (Index == 7)?'-':' ';
            Str[Index] = (c < ' ' || c > 'z') ? '.' : c;
        }

        Val[Index*3] = 0;
        Str[Index] = 0;
        Print (L"%*a%X: %-.48a *%a*\n", Indent, "", Offset, Val, Str);

        Data += Size;
        Offset += Size;
        DataSize -= Size;

        ScreenCount++;
        if (ScreenCount >= ScreenSize && ScreenSize != 0) {

            ScreenCount = 0;
            Print (L"Press Enter to continue :");
            Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
            Print (L"\n");
        }

    }
}
