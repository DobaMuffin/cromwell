#ifndef BOOTVIDEOHELPERS_H_
#define BOOTVIDEOHELPERS_H_


#include "boot.h"
#include "video.h"
#include "fontx8.h"
#include <stdarg.h>
#include "decode-jpg.h"
#include "BootVideoHelpers.h"


unsigned int BootVideoGetCharacterWidth(u8 bCharacter, bool fDouble);
unsigned int BootVideoGetStringTotalWidth(const char * szc);
unsigned int BootVideoFontWidthToBitmapBytecount(unsigned int uiWidth);
void BootVideoJpegBlitBlend(
    u8 *pDst,
    u32 dst_width,
    JPEG * pJpeg,
    u8 *pFront,
    RGBA m_rgbaTransparent,
    u8 *pBack,
    int x,
    int y
);
int BootVideoOverlayCharacter(
    u32 * pdwaTopLeftDestination,
    u32 m_dwCountBytesPerLineDestination,
    RGBA rgbaColourAndOpaqueness,
    u8 bCharacter,
    bool fDouble
);
int BootVideoOverlayString(u32 * pdwaTopLeftDestination, u32 m_dwCountBytesPerLineDestination, RGBA rgbaOpaqueness, const char * szString);
bool BootVideoJpegUnpackAsRgb(u8 *pbaJpegFileImage, JPEG * pJpeg);
u8 * BootVideoGetPointerToEffectiveJpegTopLeft(JPEG * pJpeg);
void BootVideoClearScreen(JPEG *pJpeg, int nStartLine, int nEndLine) ;
int VideoDumpAddressAndData(u32 dwAds, const u8 * baData, u32 dwCountBytesUsable);
void BootVideoChunkedPrint(const char * szBuffer);
int printk(const char *szFormat, ...);
int console_putchar(int c);


#endif