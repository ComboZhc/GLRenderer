#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

BYTE* loadBitmapFile(const char* filename,
                     BITMAPFILEHEADER* bitmapFileHeader,
                     BITMAPINFOHEADER* bitmapInfoHeader);
int saveBitmapFile(const char* filename,
                    BITMAPFILEHEADER* bitmapFileHeader,
                    BITMAPINFOHEADER* bitmapInfoHeader,
                    BYTE* image);

#endif // BITMAP_H

