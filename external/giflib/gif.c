#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#ifdef _WIN32
#include <malloc.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include "gif_lib.h"

/**
 * Decoding error - no frames
 */
#define D_GIF_ERR_NO_FRAMES     	1000
#define D_GIF_ERR_INVALID_SCR_DIMS 	1001
#define D_GIF_ERR_INVALID_IMG_DIMS 	1002
#define D_GIF_ERR_IMG_NOT_CONFINED 	1003

static void packARGB32(argb* pixel, GifByteType alpha, GifByteType red,
		GifByteType green, GifByteType blue)
{
	pixel->alpha = alpha;
	pixel->red = red;
	pixel->green = green;
	pixel->blue = blue;
}

static void getColorFromTable(int idx, argb* dst, const ColorMapObject* cmap)
{
	int colIdx = (idx >= cmap->ColorCount) ? 0 : idx;
	GifColorType* col = &cmap->Colors[colIdx];
	packARGB32(dst, 0xFF, col->Red, col->Green,col->Blue);
}

static void eraseColor(argb* bm, int w, int h, argb color)
{
	int i;
	for (i = 0; i < w * h; i++)
		*(bm + i) = color;
}

static int setupBackupBmp(GifInfo* info, int transpIndex)
{
	GifFileType* fGIF = info->gifFilePtr;
	argb paintingColor;
	info->backupPtr = (argb *)calloc(fGIF->SWidth * fGIF->SHeight, sizeof(argb));
	if (!info->backupPtr)
	{
		info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
		return FALSE;
	}
	
	if (transpIndex == -1)
		getColorFromTable(fGIF->SBackGroundColor, &paintingColor,
				fGIF->SColorMap);
	else
		packARGB32(&paintingColor, 0, 0, 0, 0);
	eraseColor(info->backupPtr, fGIF->SWidth, fGIF->SHeight, paintingColor);
	return TRUE;
}

static int getComment(GifByteType* Bytes, char** cmt)
{
	unsigned int len = (unsigned int) Bytes[0];
	unsigned int offset = *cmt != NULL ? strlen(*cmt) : 0;
	char* ret = (char *)realloc(*cmt, (len + offset + 1) * sizeof(char));
	if (ret != NULL)
	{
		memcpy(ret + offset, &Bytes[1], len);
		ret[len + offset] = 0;
		*cmt = ret;
		return GIF_OK;
	}
	return GIF_ERROR;
}

static int readExtensions(int ExtFunction, GifByteType *ExtData, GifInfo* info)
{
	char *b;
	short delay;

	if (ExtData == NULL)
		return GIF_OK;
	if (ExtFunction == GRAPHICS_EXT_FUNC_CODE && ExtData[0] == 4)
	{
		FrameInfo* fi = &info->infos[info->gifFilePtr->ImageCount];
		fi->transpIndex = -1;
		b = (char*) ExtData + 1;
		delay = ((b[2] << 8) | b[1]);
		fi->duration = delay > 1 ? delay * 10 : 100;
		fi->disposalMethod = ((b[0] >> 2) & 7);
		if (ExtData[1] & 1)
			fi->transpIndex = 0xff&b[3];
		if (fi->disposalMethod == 3 && info->backupPtr == NULL)
		{
			if (!setupBackupBmp(info, fi->transpIndex))
				return GIF_ERROR;
		}
	}
	else if (ExtFunction == COMMENT_EXT_FUNC_CODE)
	{
		if (getComment(ExtData, &info->comment) == GIF_ERROR)
		{
			info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
			return GIF_ERROR;
		}
	}
	else if (ExtFunction == APPLICATION_EXT_FUNC_CODE && ExtData[0] == 11)
	{
		if (strncmp("NETSCAPE2.0", (const char *)&ExtData[1], 11) == 0
				|| strncmp("ANIMEXTS1.0",(const char *)&ExtData[1], 11) == 0)
		{
			if (DGifGetExtensionNext(info->gifFilePtr, &ExtData,
					&ExtFunction)==GIF_ERROR)
				return GIF_ERROR;
			if (ExtFunction == APPLICATION_EXT_FUNC_CODE && ExtData[0] == 3
					&& ExtData[1] == 1)
			{
				info->loopCount = (unsigned short) (ExtData[2]
						+ (ExtData[3] << 8));
			}
		}
	}
	return GIF_OK;
}

int DDGifSlurp(GifFileType *GifFile, GifInfo* info, int shouldDecode)
{
	GifRecordType RecordType;
	GifByteType *ExtData;
	int codeSize;
	int ExtFunction;
	size_t ImageSize;
	int i;
	SavedImage* sp;
	int ok;

	do
	{
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR)
			return (GIF_ERROR);
		switch (RecordType)
		{
		case IMAGE_DESC_RECORD_TYPE:

			if (DGifGetImageDesc(GifFile, !shouldDecode) == GIF_ERROR)
				return (GIF_ERROR);
			i = shouldDecode ? info->currentIndex : GifFile->ImageCount - 1;
			sp = &GifFile->SavedImages[i];
			ImageSize = sp->ImageDesc.Width * sp->ImageDesc.Height;

			if (sp->ImageDesc.Width < 1 || sp->ImageDesc.Height < 1
					|| ImageSize > (SIZE_MAX / sizeof(GifPixelType)))
			{
				GifFile->Error = D_GIF_ERR_INVALID_IMG_DIMS;
				return GIF_ERROR;
			}
			if (sp->ImageDesc.Width > GifFile->SWidth
					|| sp->ImageDesc.Height > GifFile->SHeight)
			{
				GifFile->Error = D_GIF_ERR_IMG_NOT_CONFINED;
				return GIF_ERROR;
			}
			if (shouldDecode)
			{

				sp->RasterBits = info->rasterBits;

				if (sp->ImageDesc.Interlace)
				{
					int i, j;
					/*
					 * The way an interlaced image should be read -
					 * offsets and jumps...
					 */
					int InterlacedOffset[] =
					{ 0, 4, 2, 1 };
					int InterlacedJumps[] =
					{ 8, 8, 4, 2 };
					/* Need to perform 4 passes on the image */
					for (i = 0; i < 4; i++)
						for (j = InterlacedOffset[i]; j < sp->ImageDesc.Height;
								j += InterlacedJumps[i])
						{
							if (DGifGetLine(GifFile,
									sp->RasterBits + j * sp->ImageDesc.Width,
									sp->ImageDesc.Width) == GIF_ERROR)
								return GIF_ERROR;
						}
				}
				else
				{
					if (DGifGetLine(GifFile, sp->RasterBits,
							ImageSize) == GIF_ERROR)
						return (GIF_ERROR);
				}
				if (info->currentIndex >= GifFile->ImageCount - 1)
				{
					if (info->loopCount > 0)
						info->currentLoop++;
					if (info->rewindFunc(info) != 0)
					{
						info->gifFilePtr->Error = D_GIF_ERR_READ_FAILED;
						return GIF_ERROR;
					}
				}
				return GIF_OK;
			}
			else
			{
				if (DGifGetCode(GifFile, &codeSize, &ExtData) == GIF_ERROR)
					return (GIF_ERROR);
				while (ExtData != NULL)
				{
					if (DGifGetCodeNext(GifFile, &ExtData) == GIF_ERROR)
						return (GIF_ERROR);
				}
			}
			break;

		case EXTENSION_RECORD_TYPE:
			if (DGifGetExtension(GifFile, &ExtFunction, &ExtData) == GIF_ERROR)
				return (GIF_ERROR);

			if (!shouldDecode)
			{
				info->infos = (FrameInfo *)realloc(info->infos,
						(GifFile->ImageCount + 1) * sizeof(FrameInfo));

				if (readExtensions(ExtFunction, ExtData, info) == GIF_ERROR)
					return GIF_ERROR;
			}
			while (ExtData != NULL)
			{
				if (DGifGetExtensionNext(GifFile, &ExtData,
						&ExtFunction) == GIF_ERROR)
					return (GIF_ERROR);
				if (!shouldDecode)
				{
					if (readExtensions(ExtFunction, ExtData, info) == GIF_ERROR)
						return GIF_ERROR;
				}
			}
			break;

		case TERMINATE_RECORD_TYPE:
			break;

		default: /* Should be trapped by DGifGetRecordType */
			break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);
	ok = TRUE;
	if (shouldDecode)
	{
		ok = (info->rewindFunc(info) == 0);
	}
	if (ok)
		return (GIF_OK);
	else
	{
		info->gifFilePtr->Error = D_GIF_ERR_READ_FAILED;
		return (GIF_ERROR);
	}
}
