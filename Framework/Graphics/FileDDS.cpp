#include "FileDDS.h"



HRESULT LoadDDS(cstring filePath, DDS_Header *pHeader, std::unique_ptr<byte[]>& pixels)
{
	FileAutoClose hf;
	errno_t res = fopen_s(hf.getAddrOf(), filePath.c_str(), "rb");
	if (res != 0 )
		return res;

	fread(pHeader, sizeof(DDS_Header), 1, hf);

	if (pHeader->symbolsDDS != DDS_MAGIC)
		return ERROR_INVALID_DATA;

	if ( (pHeader->size+4) != sizeof(DDS_Header) || pHeader->ddspf.size != sizeof(DDS_PixelFormat) )
		return ERROR_INVALID_DATA;

	UINT bits_per_pixel = pHeader->ddspf.RGBBitCount;  // may be less then 8 bits.
	UINT image_size = 0;

	if (pHeader->mipMapCount > 1)
	{
	}
	else
	{
		image_size = ( pHeader->width * bits_per_pixel ) / 8 * pHeader->height;
	}

	if (image_size)
	{
		pixels = std::make_unique<byte[]>(image_size);
		if (pixels.get())
		{
			size_t count = fread_s(pixels.get(), image_size, image_size, 1, hf);
			if (count != 1)
				return ERROR_FILE_INVALID;
		}
	}
	else
		return ERROR_INVALID_DATA;

	return S_OK;
}