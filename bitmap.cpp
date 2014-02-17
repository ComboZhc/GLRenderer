#include "bitmap.hpp"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

BYTE *loadBitmapFile(const char *filename,
                     BITMAPFILEHEADER *bitmapFileHeader,
                     BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    BYTE *bitmapImage;  //store image data
    int imageIdx;  //image index counter
    int paddingBytes;
    int imageSize;
    BYTE tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader->bfType != 0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr); // small edit. forgot to add the closing bracket at sizeof

    if (bitmapInfoHeader->biCompression != 0) {
        fclose(filePtr);
        return NULL;
    }

    imageSize = bitmapInfoHeader->biWidth * bitmapInfoHeader->biHeight * 3;

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (BYTE*)malloc(imageSize);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }


    //read in the bitmap image data
    fread(bitmapImage, imageSize, 1, filePtr);

    //remove padding bits
    paddingBytes = bitmapInfoHeader->biWidth % 4 ? 4 - bitmapInfoHeader->biWidth % 4 : 0;
    if (paddingBytes > 0)
        for (imageIdx = 0; imageIdx < bitmapInfoHeader->biHeight; ++imageIdx) {
            memmove(&bitmapImage[imageIdx * bitmapInfoHeader->biWidth * 3],
                    &bitmapImage[imageIdx * bitmapInfoHeader->biWidth * 3 + imageIdx * paddingBytes],
                    imageIdx * bitmapInfoHeader->biWidth * 3);
        }

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;
         imageIdx < imageSize;
         imageIdx += 3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return bitmapImage;
}

int saveBitmapFile(const char* filename,
                    BITMAPFILEHEADER* bitmapFileHeader,
                    BITMAPINFOHEADER* bitmapInfoHeader,
                    BYTE* image) {
    //open filename in write binary mode
    FILE* filePtr;
    filePtr = fopen(filename,"wb");
    if (filePtr == NULL)
        return -1;
    fwrite(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    fwrite(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);
    fwrite(image, bitmapInfoHeader->biWidth * bitmapInfoHeader->biHeight * 3, 1, filePtr);
    fclose(filePtr);
    return 0;
}
