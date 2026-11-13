#include "libegint.h"
#include "../refind/global.h"

#include "egemb_font.h"
#include "egemb_font_large.h"
#define FONT_NUM_CHARS 96
#include "zh_glyphs.h"
static BOOLEAN ZhFontLoaded = FALSE;

static EG_IMAGE *BaseFontImage = NULL;
static EG_IMAGE *DarkFontImage = NULL;
static EG_IMAGE *LightFontImage = NULL;

static UINTN FontCellWidth = 7;

static VOID egPrepareFont() {
    UINTN UGAWidth, UGAHeight;

    egGetScreenSize(&UGAWidth, &UGAHeight);
    if (BaseFontImage == NULL) {
        if (UGAWidth >= HIDPI_MIN)
            BaseFontImage = egPrepareEmbeddedImage(&egemb_font_large, TRUE);
        else
            BaseFontImage = egPrepareEmbeddedImage(&egemb_font, TRUE);
    }
    if (BaseFontImage != NULL)
        FontCellWidth = BaseFontImage->Width / (ZhFontLoaded ? ZH_FONT_NUM_CHARS : FONT_NUM_CHARS);
}

static UINTN GlyphAdvance(CHAR16 c) {
    return ZhFontLoaded && c >= 32 && c < 127 ? 10 : FontCellWidth;
}

UINTN egGetFontHeight(VOID) {
   egPrepareFont();
   return BaseFontImage->Height;
}

UINTN egGetFontCellWidth(VOID) {
   return ZhFontLoaded ? 10 : FontCellWidth;
}

UINTN egComputeTextWidth(IN CHAR16 *Text) {
   UINTN Width = 0;

   egPrepareFont();
   if (Text != NULL)
      {
      UINTN n;
      for (n = 0; Text[n]; n++) Width += GlyphAdvance(Text[n]);
   }
   return Width;
}

VOID egMeasureText(IN CHAR16 *Text, OUT UINTN *Width, OUT UINTN *Height) {
    egPrepareFont();

    if (Width != NULL)
        *Width = egComputeTextWidth(Text);
    if (Height != NULL)
        *Height = BaseFontImage->Height;
}

VOID egRenderText(IN CHAR16 *Text, IN OUT EG_IMAGE *CompImage, IN UINTN PosX, IN UINTN PosY, IN UINT8 BGBrightness)
{
    EG_IMAGE        *FontImage;
    EG_PIXEL        *BufferPtr;
    EG_PIXEL        *FontPixelData;
    UINTN           BufferLineOffset, FontLineOffset;
    UINTN           TextLength;
    UINTN           i, c;

    egPrepareFont();

    if (Text)
       TextLength = StrLen(Text);
    else
       TextLength = 0;

    if (PosX >= CompImage->Width || PosY >= CompImage->Height) return;

    if (BGBrightness < 128) {
       if (LightFontImage == NULL) {
          LightFontImage = egCopyImage(BaseFontImage);
          if (LightFontImage == NULL)
             return;
          for (i = 0; i < (LightFontImage->Width * LightFontImage->Height); i++) {
             LightFontImage->PixelData[i].r = 255 - LightFontImage->PixelData[i].r;
             LightFontImage->PixelData[i].g = 255 - LightFontImage->PixelData[i].g;
             LightFontImage->PixelData[i].b = 255 - LightFontImage->PixelData[i].b;
          }
       }
       FontImage = LightFontImage;
    } else {
       if (DarkFontImage == NULL)
          DarkFontImage = egCopyImage(BaseFontImage);
       if (DarkFontImage == NULL)
          return;
       FontImage = DarkFontImage;
    }

    BufferPtr = CompImage->PixelData;
    BufferLineOffset = CompImage->Width;
    BufferPtr += PosX + PosY * BufferLineOffset;
    FontPixelData = FontImage->PixelData;
    FontLineOffset = FontImage->Width;
    for (i = 0; i < TextLength; i++) {
        UINTN advance = GlyphAdvance(Text[i]);
        UINTN drawnHeight = FontImage->Height;
        if (advance > CompImage->Width - PosX) break;
        if (drawnHeight > CompImage->Height - PosY) drawnHeight = CompImage->Height - PosY;
        c = Text[i];
        if (c >= 32 && c < 127) {
            c -= 32;
        } else {
            UINTN low = 0, high = sizeof(ZhGlyphs) / sizeof(ZhGlyphs[0]);
            UINTN glyph = 95;
            if (ZhFontLoaded) {
                while (low < high) {
                    UINTN mid = low + (high - low) / 2;
                    if (ZhGlyphs[mid] < c) low = mid + 1;
                    else high = mid;
                }
                if (low < sizeof(ZhGlyphs) / sizeof(ZhGlyphs[0]) && ZhGlyphs[low] == c)
                    glyph = 96 + low;
            }
            c = glyph;
        }
        egRawCompose(BufferPtr, FontPixelData + c * FontCellWidth + (FontCellWidth - advance) / 2,
                     advance, drawnHeight,
                     BufferLineOffset, FontLineOffset);
        BufferPtr += advance;
        PosX += advance;
    }
}

VOID egLoadFont(IN CHAR16 *Filename) {
   if (BaseFontImage)
      egFreeImage(BaseFontImage);

   if (DarkFontImage) { egFreeImage(DarkFontImage); DarkFontImage = NULL; }
   if (LightFontImage) { egFreeImage(LightFontImage); LightFontImage = NULL; }
   BaseFontImage = egLoadImage(SelfDir, Filename, TRUE);
   ZhFontLoaded = BaseFontImage && BaseFontImage->Width == ZH_FONT_NUM_CHARS * 16 && BaseFontImage->Height == 24;
   if (BaseFontImage == NULL)
      Print(L"Note: Font image file %s is invalid! Using default font!\n");
    egPrepareFont();
}
