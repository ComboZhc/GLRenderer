#ifndef BITMAP_H
#define BITMAP_H

#include <windows.h>

BYTE* loadBitmapFile(const char* filename,
                     BITMAPFILEHEADER* bitmapFileHeader,
                     BITMAPINFOHEADER* bitmapInfoHeader);
int saveBitmapFile(const char* filename,
                    BITMAPFILEHEADER* bitmapFileHeader,
                    BITMAPINFOHEADER* bitmapInfoHeader,
                    BYTE* image);

#endif // BITMAP_H

