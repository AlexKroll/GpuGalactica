#pragma once

#include <windows.h>
#include <string>
#include <memory>

#include "../Defines.h"
#include "../Utils/Utils.h"


struct DDS_PixelFormat
{
    uint32_t  size;
    uint32_t  flags;
    uint32_t  fourCC;
    uint32_t  RGBBitCount;
    uint32_t  RBitMask;
    uint32_t  GBitMask;
    uint32_t  BBitMask;
    uint32_t  ABitMask;
};


struct DDS_Header
{
	uint32_t  symbolsDDS;
    uint32_t  size;
    uint32_t  flags;
    uint32_t  height;
    uint32_t  width;
    uint32_t  pitchOrLinearSize;
    uint32_t  depth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
    uint32_t  mipMapCount;
    uint32_t  reserved1[11];
    DDS_PixelFormat ddspf;
    uint32_t  surfaceFlags;
    uint32_t  cubemapFlags;
    uint32_t  reserved2[3];
};


#define DDS_MAGIC 0x20534444		// DirectX DDS files must begin with the string "DDS ".


HRESULT LoadDDS(cstring filePath, DDS_Header *pHeader, std::unique_ptr<byte[]>& pixels);