#include "ImageToHBITMAP.h"


using namespace std;

HBITMAP createHBITMAPFromImage(LPBYTE pImage, DWORD sizeImage)
{
	CComPtr<IWICImagingFactory> pFactory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID *)&pFactory);
	if (FAILED(hr))
		return NULL;

	CComPtr<IWICStream> pStream;
	hr = pFactory->CreateStream(&pStream);
	if (FAILED(hr))
		return NULL;

	hr = pStream->InitializeFromMemory(pImage, sizeImage);
	if (FAILED(hr))
		return NULL;

	unique_ptr<BYTE[]> pBitmapImageWithFileHeader;
	CComPtr<IWICBitmapDecoder> pDecoder;
	hr = pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeOptions::WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr))
	{
		// It is probably bitmap format without it's file header, hence the image format
		//  identification failed. To fix/test this, we create a file header ourselves and
		//  and point the stream to this header.
		pBitmapImageWithFileHeader.reset(new BYTE[sizeof(BITMAPFILEHEADER) + sizeImage]);

		PBITMAPFILEHEADER pheaderBitmapFile = PBITMAPFILEHEADER(pBitmapImageWithFileHeader.get());
		pheaderBitmapFile->bfType = 'MB'; // Bitmap signature 'BM' in little endian
		pheaderBitmapFile->bfSize = sizeof(BITMAPFILEHEADER) + sizeImage;
		pheaderBitmapFile->bfReserved1 = 0;
		pheaderBitmapFile->bfReserved2 = 0;
		pheaderBitmapFile->bfOffBits = sizeof(BITMAPFILEHEADER) + *LPDWORD(pImage);
		CopyMemory(pBitmapImageWithFileHeader.get() + sizeof(BITMAPFILEHEADER), pImage, sizeImage);

		pStream = NULL;
		hr = pFactory->CreateStream(&pStream);
		if (FAILED(hr))
			return NULL;

		hr = pStream->InitializeFromMemory(pBitmapImageWithFileHeader.get(), sizeof(BITMAPFILEHEADER) + sizeImage);
		if (FAILED(hr))
			return NULL;

		hr = pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeOptions::WICDecodeMetadataCacheOnLoad, &pDecoder);
		if (FAILED(hr))
			return NULL;

		/*hr = pFactory->CreateDecoder(GUID_ContainerFormatBmp, NULL, &pDecoder);
		if (FAILED(hr))
			return NULL;

		hr = pDecoder->Initialize(pStream, WICDecodeOptions::WICDecodeMetadataCacheOnDemand);
		if (FAILED(hr))
			return NULL;*/
	}

	UINT Count = 0;
	hr = pDecoder->GetFrameCount(&Count);
	if (FAILED(hr) || Count == 0)
		return NULL;

	CComPtr<IWICBitmapFrameDecode> pBitmapFrame;
	hr = pDecoder->GetFrame(0, &pBitmapFrame);
	if (FAILED(hr))
		return NULL;

	WICPixelFormatGUID PixelFormatGUID;
	hr = pBitmapFrame->GetPixelFormat(&PixelFormatGUID);
	if (FAILED(hr))
		return NULL;

	CComPtr<IWICBitmapSource> pDest;
	if (!IsEqualGUID(PixelFormatGUID, GUID_WICPixelFormat32bppPBGRA))
	{
		// We need to convert this image to proper format
		IWICBitmapSource *pSource = pBitmapFrame;

		hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, pSource, &pDest);
		if (FAILED(hr))
			return NULL;
	}
	else
		pDest = pBitmapFrame;

	CComPtr<IWICBitmap> pBitmap;
	hr = pFactory->CreateBitmapFromSource(pDest, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &pBitmap);
	if (FAILED(hr))
		return NULL;

	UINT uiWidth = 0, uiHeight = 0;
	hr = pBitmap->GetSize(&uiWidth, &uiHeight);
	if (FAILED(hr))
		return NULL;

	WICRect rectImage = { 0, 0, uiWidth, uiHeight };
	CComPtr<IWICBitmapLock> pLock;
	hr = pBitmap->Lock(&rectImage, WICBitmapLockRead, &pLock);
	if (FAILED(hr))
		return NULL;

	UINT sizeImageBuffer = 0;
	LPBYTE pImageBuffer;
	hr = pLock->GetDataPointer(&sizeImageBuffer, &pImageBuffer);
	if (FAILED(hr))
		return NULL;

	/*unique_ptr<BYTE> pImageBuffer(new BYTE[uiWidth * uiHeight * sizeof(DWORD)]);
	hr = pDest->CopyPixels(&rectImage, uiWidth * sizeof(DWORD), uiWidth * uiHeight * sizeof(DWORD), pImageBuffer.get());
	if (FAILED(hr))
		return NULL;*/

	HBITMAP hBitmap = CreateBitmap(uiWidth, uiHeight, 1, 32, pImageBuffer);
	assert(hBitmap);

	return hBitmap;
}