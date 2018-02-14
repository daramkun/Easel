#define EAGEL_D3D11
#include <easel.h>

#include <atlconv.h>
#include <atlbase.h>
#include <wincodec.h>
#pragma comment ( lib, "windowscodecs.lib" )

bool LoadTexture2D ( const char * filename, ID3D11Device * d3dDevice, ID3D11Texture2D ** result )
{
	USES_CONVERSION;

	CComPtr<IWICImagingFactory> imagingFactory;
	if ( FAILED ( CoCreateInstance ( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, ( LPVOID* ) &imagingFactory ) ) )
		return false;

	CComPtr<IWICBitmapDecoder> bitmapDecoder;
	if ( FAILED ( imagingFactory->CreateDecoderFromFilename ( A2W ( filename ), nullptr,
		GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder ) ) )
		return false;

	CComPtr<IWICBitmapFrameDecode> bitmapFrame;
	if ( FAILED ( bitmapDecoder->GetFrame ( 0, &bitmapFrame ) ) )
		return false;

	CComPtr<IWICFormatConverter> formatConverter;
	if ( FAILED ( imagingFactory->CreateFormatConverter ( &formatConverter ) ) )
		return false;
	if ( FAILED ( formatConverter->Initialize ( bitmapFrame,
		GUID_WICPixelFormat32bppBGRA,
		WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom ) ) )
		return false;

	D3D11_TEXTURE2D_DESC texDesc = { 0, };
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	bitmapFrame->GetSize ( &texDesc.Width, &texDesc.Height );
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	byte * copyMemory = new byte [ texDesc.Width * texDesc.Height * 4 ];
	if ( FAILED ( formatConverter->CopyPixels ( nullptr,
		texDesc.Width * 4,
		texDesc.Width * texDesc.Height * 4,
		copyMemory ) ) )
	{
		delete [] copyMemory;
		return false;
	}

	D3D11_SUBRESOURCE_DATA initialData = { 0, };
	initialData.pSysMem = copyMemory;
	initialData.SysMemPitch = texDesc.Width * 4;
	initialData.SysMemSlicePitch = texDesc.Width * 4 * texDesc.Height;
	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, &initialData, result ) ) )
	{
		delete [] copyMemory;
		return false;
	}

	delete [] copyMemory;

	return true;
}

bool CreateBlankTexture2D ( unsigned width, unsigned height, ID3D11Device * d3dDevice, ID3D11Texture2D ** result )
{
	D3D11_TEXTURE2D_DESC texDesc = { 0, };
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, nullptr, result ) ) )
	{
		return false;
	}

	return true;
}

bool SaveTexture2D ( const char * filename, ID3D11Texture2D * texture, ID3D11Device * d3dDevice )
{
	USES_CONVERSION;

	CComPtr<IWICImagingFactory> imagingFactory;
	if ( FAILED ( CoCreateInstance ( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, ( LPVOID* ) &imagingFactory ) ) )
		return false;

	CComPtr<IWICBitmapEncoder> bitmapEncoder;
	if ( FAILED ( imagingFactory->CreateEncoder ( GUID_ContainerFormatPng,
		nullptr, &bitmapEncoder ) ) )
		return false;

	CComPtr<IStream> fileStream;
	if ( FAILED ( SHCreateStreamOnFile ( A2W ( filename ),
		STGM_WRITE | STGM_CREATE, &fileStream ) ) )
		return false;

	if ( FAILED ( bitmapEncoder->Initialize ( fileStream, WICBitmapEncoderNoCache ) ) )
		return false;

	CComPtr<IWICBitmapFrameEncode> frameEncode;
	CComPtr<IPropertyBag2> encoderOptions;
	if ( FAILED ( bitmapEncoder->CreateNewFrame ( &frameEncode, &encoderOptions ) ) )
		return false;
	if ( FAILED ( frameEncode->Initialize ( nullptr ) ) )
		return false;

	D3D11_TEXTURE2D_DESC texDesc;
	texture->GetDesc ( &texDesc );

	if ( FAILED ( frameEncode->SetResolution ( 96, 96 ) ) )
		return false;
	if ( FAILED ( frameEncode->SetSize ( texDesc.Width, texDesc.Height ) ) )
		return false;
	GUID encodeGuid = GUID_WICPixelFormat32bppBGRA;
	if ( FAILED ( frameEncode->SetPixelFormat ( &encodeGuid ) ) )
		return false;

	texDesc.BindFlags = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	CComPtr<ID3D11Texture2D> lockTexture;
	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, nullptr, &lockTexture ) ) )
		return false;
	CComPtr<ID3D11DeviceContext> immediateContext;
	d3dDevice->GetImmediateContext ( &immediateContext );
	immediateContext->CopyResource ( lockTexture, texture );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if ( FAILED ( immediateContext->Map ( lockTexture, 0, D3D11_MAP_READ, 0, &mappedResource ) ) )
		return false;
	if ( FAILED ( frameEncode->WritePixels ( texDesc.Height,
		texDesc.Width * 4,
		texDesc.Width * texDesc.Height * 4,
		( BYTE* ) mappedResource.pData ) ) )
		return false;
	immediateContext->Unmap ( lockTexture, 0 );

	if ( FAILED ( frameEncode->Commit () ) )
		return false;
	if ( FAILED ( bitmapEncoder->Commit () ) )
		return false;

	return true;
}

int DoProgram ()
{
	CComPtr<ID3D11Device> d3dDevice;
	if ( egGetD3D11Device ( &d3dDevice ) != EAGELERR_SUCCEED )
		return -1;

	CComPtr<ID3D11Texture2D> sampleTexture, targetTexture, resizeTexture;
	if ( !LoadTexture2D ( "Sample.jpg", d3dDevice, &sampleTexture ) )
		return -2;
	D3D11_TEXTURE2D_DESC desc;
	sampleTexture->GetDesc ( &desc );
	if ( !CreateBlankTexture2D ( desc.Width, desc.Height, d3dDevice, &targetTexture ) )
		return -3;
	if ( !CreateBlankTexture2D ( desc.Width * 2, desc.Height * 2, d3dDevice, &resizeTexture ) )
		return -4;

	float filterValues [] = { 
		/*0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f,
		0.00002292f, 0.00078634f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f,
		0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f,
		0.00038771f, 0.01330373f, 0.11098164f, 0.22508352f, 0.11098164f, 0.01330373f, 0.00038771f,
		0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f,
		0.00002292f, 0.00078633f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f,
		0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f,*/
		0, -2, 0,
		-2, 11, -2,
		0, -2, 0
	};
	EAGEL_FILTER filter/* ( filterValues,
		( int ) sqrt ( _countof ( filterValues ) ) / 2,
		( int ) sqrt ( _countof ( filterValues ) ) / 2 );
	filter.divide ( 3 )*/ = EAGEL_FILTER::getSharpenFilter ();
	if ( egDoFiltering ( targetTexture, sampleTexture, filter ) != EAGELERR_SUCCEED )
		return -5;

	if ( egDoResize ( resizeTexture, targetTexture, EAGEL_SAMPLING_NEAREST ) )
		return -6;

	if ( !SaveTexture2D ( "Result.png", resizeTexture, d3dDevice ) )
		return -7;

	return 0;
}

int main ( int argc, char ** argv )
{
	CoInitialize ( nullptr );
	egInitializeD3D11 ();

	int ret = DoProgram ();

	egUninitializeD3D11 ();
	CoUninitialize ();

	return ret;
}