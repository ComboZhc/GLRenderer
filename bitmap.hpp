#ifndef BITMAP_H
#define BITMAP_H

#include <windows.h>

BYTE *loadBitmapFile(const char *filename,
                     BITMAPFILEHEADER *bitmapFileHeader,
                     BITMAPINFOHEADER *bitmapInfoHeader);

#endif // BITMAP_H

