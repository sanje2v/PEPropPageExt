#pragma once

#include <memory>
#include <cassert>
#include <atlbase.h>
#include <wincodec.h>

HBITMAP createHBITMAPFromImage(LPBYTE pImage, DWORD sizeImage);