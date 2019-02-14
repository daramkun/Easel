#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11Bitmap::D3D11Bitmap ( D3D11EaselFactory * easelFactory, ID3D11Texture2D * texture, ID3D11ShaderResourceView * srv,
	ID3D11UnorderedAccessView * uav, ID3D11RenderTargetView * rtv )
	: _refCount ( 1 ), _factory ( easelFactory )
	, _texture ( texture ), _srv ( srv ), _uav ( uav ), _rtv ( rtv )
{ }

HRESULT __stdcall easel::d3d11::D3D11Bitmap::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::Bitmap ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11Bitmap::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11Bitmap::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

void easel::d3d11::D3D11Bitmap::GetSize ( UINT * width, UINT * height )
{
	D3D11_TEXTURE2D_DESC desc;
	_texture->GetDesc ( &desc );

	if ( width ) *width = desc.Width;
	if ( height ) *height = desc.Height;
}

HRESULT easel::d3d11::D3D11Bitmap::Encode ( IStream * stream, float quality )
{
	D3D11_TEXTURE2D_DESC desc;
	_texture->GetDesc ( &desc );

	CComPtr<IWICBitmapEncoder> encoder;
	if ( FAILED ( _factory->_wicImagingFactory->CreateEncoder ( GUID_ContainerFormatJpeg, nullptr, &encoder ) ) )
		return E_FAIL;

	if ( FAILED ( encoder->Initialize ( stream, WICBitmapEncoderNoCache ) ) )
		return E_FAIL;

	CComPtr<IWICBitmapFrameEncode> frameEncode;
	CComPtr<IPropertyBag2> encoderOptions;
	if ( FAILED ( encoder->CreateNewFrame ( &frameEncode, &encoderOptions ) ) )
		return E_FAIL;

	PROPBAG2 propBag2 = { 0 };
	VARIANT variant;

	propBag2.pstrName = ( LPOLESTR ) L"ImageQuality";
	VariantInit ( &variant );
	variant.vt = VT_R4;
	variant.fltVal = quality;
	if ( FAILED ( encoderOptions->Write ( 0, &propBag2, &variant ) ) )
		return E_FAIL;

	propBag2.pstrName = ( LPOLESTR ) L"JpegYCrCbSubsampling";
	VariantInit ( &variant );
	variant.vt = VT_UI1;
	variant.bVal = WICJpegYCrCbSubsampling420;
	if ( FAILED ( encoderOptions->Write ( 1, &propBag2, &variant ) ) )
		return E_FAIL;

	propBag2.pstrName = ( LPOLESTR ) L"SuppressApp0";
	VariantInit ( &variant );
	variant.vt = VT_BOOL;
	variant.intVal = FALSE;
	if ( FAILED ( encoderOptions->Write ( 1, &propBag2, &variant ) ) )
		return E_FAIL;

	if ( FAILED ( frameEncode->Initialize ( encoderOptions ) ) )
		return E_FAIL;

	if ( FAILED ( frameEncode->SetResolution ( 96, 96 ) ) )
		return E_FAIL;
	if ( FAILED ( frameEncode->SetSize ( desc.Width, desc.Height ) ) )
		return E_FAIL;
	WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat128bppRGBAFloat;
	if ( FAILED ( frameEncode->SetPixelFormat ( &pixelFormat ) ) )
		return E_FAIL;

	CComPtr<ID3D11Texture2D> copyTex;
	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	if ( FAILED ( _factory->_d3dDevice->CreateTexture2D ( &desc, nullptr, &copyTex ) ) )
		return E_FAIL;

	_factory->_immediateContext->CopyResource ( copyTex, _texture );

	D3D11_MAPPED_SUBRESOURCE subres;
	if ( SUCCEEDED ( _factory->_immediateContext->Map ( copyTex, 0, D3D11_MAP_READ, 0, &subres ) ) )
	{
		CComPtr<IWICBitmap> bitmap;
		if ( SUCCEEDED ( _factory->_wicImagingFactory->CreateBitmap ( Width (), Height (),
			GUID_WICPixelFormat128bppRGBAFloat, WICBitmapCacheOnDemand, &bitmap ) ) )
		{
			CComPtr<IWICBitmapLock> lock;
			if ( SUCCEEDED ( bitmap->Lock ( nullptr, WICBitmapLockWrite, &lock ) ) )
			{
				WICInProcPointer buffer;
				UINT bufferSize;
				if ( SUCCEEDED ( lock->GetDataPointer ( &bufferSize, &buffer ) ) )
					for ( unsigned int y = 0; y < Height (); ++y )
						memcpy ( buffer + ( y * ( Width () * 16 ) ), ( ( BYTE * ) subres.pData ) + y * subres.RowPitch, Width () * 16 );
			}
		}

		if ( FAILED ( frameEncode->WriteSource ( bitmap, nullptr ) ) )
			return E_FAIL;

		_factory->_immediateContext->Unmap ( copyTex, 0 );
	}

	frameEncode->Commit ();
	encoder->Commit ();

	return S_OK;
}