#include "easel.h"

#if EASEL_D3D11

#include <atlbase.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#pragma comment ( lib, "d3d11.lib" )
#pragma comment ( lib, "d3dcompiler.lib" )
#pragma comment ( lib, "WindowsCodecs.lib" )

#include <string>
#include <sstream>

#define SAFE_RELEASE(x)										if ( x ) x->Release (); x = nullptr;

#include "__crc32.h"
#include "__shader.d3d11.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned g_processingUnitD3D11;

ID3D11Device * g_d3dDevice;
ID3D11DeviceContext * g_immediateContext;

std::map<ID3D11Texture2D *, ID3D11ShaderResourceView *> g_srvsForTexture2D;
std::map<ID3D11Texture2D *, ID3D11UnorderedAccessView *> g_uavsForTexture2D;

std::map<uint32_t, ID3D11ComputeShader *> g_shadersD3D11;

ID3D11Buffer * g_filteringConstantsD3D11;
ID3D11SamplerState * g_resizeSamplerNearest, * g_resizeSamplerLinear;
ID3D11Buffer * g_histogramBufferD3D11;
ID3D11ShaderResourceView * g_histogramSRV;
ID3D11UnorderedAccessView * g_histogramUAV;

ID3D11ShaderResourceView * __egGetShaderResourceView ( ID3D11Texture2D * texture )
{
	ID3D11ShaderResourceView * srv;
	if ( g_srvsForTexture2D.find ( texture ) == g_srvsForTexture2D.end () )
	{
		if ( FAILED ( g_d3dDevice->CreateShaderResourceView ( texture, nullptr, &srv ) ) )
			return nullptr;
		g_srvsForTexture2D.insert ( std::pair<ID3D11Texture2D*, ID3D11ShaderResourceView*> ( texture, srv ) );
	}
	else srv = g_srvsForTexture2D.at ( texture );
	return srv;
}
ID3D11UnorderedAccessView * __egGetUnorderedAccessView ( ID3D11Texture2D * texture )
{
	ID3D11UnorderedAccessView * uav;
	if ( g_uavsForTexture2D.find ( texture ) == g_uavsForTexture2D.end () )
	{
		if ( FAILED ( g_d3dDevice->CreateUnorderedAccessView ( texture, nullptr, &uav ) ) )
			return nullptr;
		g_uavsForTexture2D.insert ( std::pair<ID3D11Texture2D*, ID3D11UnorderedAccessView*> ( texture, uav ) );
	}
	else uav = g_uavsForTexture2D.at ( texture );
	return uav;
}
const char * __egGetSourceFormat ( DXGI_FORMAT format )
{
	switch ( format )
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return "unorm float4";

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return "float4";

		default: return "float4";
	}
}
const char * __egGetDestinationFormat ( DXGI_FORMAT format )
{
	switch ( format )
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return "unorm float4";

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return "float4";

		default: return "float4";
	}
}
struct EGMACRO
{
public:
	EGMACRO ( unsigned processingUnit, DXGI_FORMAT sourceFormat, DXGI_FORMAT destinationFormat, const char * shaderInfo )
	{
		_itoa_s ( g_processingUnitD3D11, itoaProcessingUnit, 10 );
		macro [ 0 ] = { "SHADER_INFO", shaderInfo };
		macro [ 1 ] = { "PROCESSINGUNIT", itoaProcessingUnit };
		macro [ 2 ] = { "INPUT_FORMAT", __egGetSourceFormat ( sourceFormat ) };
		macro [ 3 ] = { "OUTPUT_FORMAT", __egGetDestinationFormat ( destinationFormat ) };
		macro [ 4 ] = { nullptr, nullptr };
	}

public:
	uint32_t getHashcode () const
	{
		std::stringstream ss;

		const D3D_SHADER_MACRO * temp = macro;
		while ( temp->Name != nullptr )
		{
			ss << temp->Name;
			ss << temp->Definition;

			++temp;
		}

		return crc32c ( 0, ( const unsigned char * ) ss.str ().c_str (), ss.str ().size () );
	}

	const D3D_SHADER_MACRO * getMacroArray () const { return macro; }

private:
	D3D_SHADER_MACRO macro [ 8 ];
	char itoaProcessingUnit [ 8 ];
};
ID3D11ComputeShader * __egGetShader ( const EGMACRO * macro )
{
	uint32_t hash = macro->getHashcode ();
	const D3D_SHADER_MACRO * macroArray = macro->getMacroArray ();
	if ( g_shadersD3D11.find ( hash ) == g_shadersD3D11.end () )
	{
		CComPtr<ID3DBlob> compiled, errMsg;
		if ( FAILED ( D3DCompile ( textureProcessingShaderSourceCode, strlen ( textureProcessingShaderSourceCode ),
			nullptr, macroArray, nullptr, macroArray [ 0 ].Definition, "cs_5_0", 0, 0, &compiled, &errMsg ) ) )
		{
			OutputDebugStringA ( ( LPCSTR ) errMsg->GetBufferPointer () );
			return nullptr;
		}

		CComPtr<ID3D11ComputeShader> computeShader;
		if ( FAILED ( g_d3dDevice->CreateComputeShader (
			compiled->GetBufferPointer (), compiled->GetBufferSize (),
			nullptr,
			&computeShader
		) ) )
			return nullptr;

		g_shadersD3D11.insert ( std::pair<uint32_t, ID3D11ComputeShader *> ( hash, computeShader ) );
		return computeShader.Detach ();
	}
	return g_shadersD3D11.at ( hash );
}
ID3D11SamplerState * __egGetSamplerState ( EASEL_SAMPLING sampling )
{
	D3D11_SAMPLER_DESC samplerDesc;
	switch ( sampling )
	{
		case EASEL_SAMPLING_NEAREST:
			if ( g_resizeSamplerNearest == nullptr )
			{
				ZeroMemory ( &samplerDesc, sizeof ( D3D11_SAMPLER_DESC ) );
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.MinLOD = -FLT_MAX;
				samplerDesc.MaxLOD = FLT_MAX;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				if ( FAILED ( g_d3dDevice->CreateSamplerState ( &samplerDesc, &g_resizeSamplerNearest ) ) )
					return nullptr;
			}
			return g_resizeSamplerNearest;

		case EASEL_SAMPLING_LINEAR:
			if ( g_resizeSamplerLinear == nullptr )
			{
				ZeroMemory ( &samplerDesc, sizeof ( D3D11_SAMPLER_DESC ) );
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.MinLOD = -FLT_MAX;
				samplerDesc.MaxLOD = FLT_MAX;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				if ( FAILED ( g_d3dDevice->CreateSamplerState ( &samplerDesc, &g_resizeSamplerLinear ) ) )
					return nullptr;
			}
			return g_resizeSamplerLinear;

		default: return nullptr;
	}
}
WICPixelFormatGUID __esConvertPixelFormatGUID ( DXGI_FORMAT format )
{
	switch ( format )
	{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return GUID_WICPixelFormat128bppRGBAFloat;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return GUID_WICPixelFormat64bppRGBAHalf;
		case DXGI_FORMAT_R16G16B16A16_UNORM: return GUID_WICPixelFormat64bppRGBA;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return GUID_WICPixelFormat32bppRGBA;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return GUID_WICPixelFormat32bppBGRA;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return GUID_WICPixelFormat32bppBGR;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return GUID_WICPixelFormat32bppRGBA1010102XR;
		case DXGI_FORMAT_R10G10B10A2_UNORM: return GUID_WICPixelFormat32bppRGBA1010102;
		case DXGI_FORMAT_B5G5R5A1_UNORM: return GUID_WICPixelFormat16bppBGRA5551;
		case DXGI_FORMAT_B5G6R5_UNORM: return GUID_WICPixelFormat16bppBGR565;
		case DXGI_FORMAT_R32_FLOAT: return GUID_WICPixelFormat32bppGrayFloat;
		case DXGI_FORMAT_R16_FLOAT: return GUID_WICPixelFormat16bppGrayHalf;
		case DXGI_FORMAT_R16_UNORM: return GUID_WICPixelFormat16bppGray;
		case DXGI_FORMAT_R8_UNORM: return GUID_WICPixelFormat8bppGray;
		default: return GUID_WICPixelFormatUndefined;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

inline void __egResetComputeShaderInContextD3D11 ()
{
	static ID3D11ShaderResourceView * srv = nullptr;
	static ID3D11UnorderedAccessView * uav = nullptr;
	static ID3D11Buffer * constantBuffer = nullptr;
	static ID3D11SamplerState * samplerState = nullptr;

	g_immediateContext->CSSetShader ( nullptr, nullptr, 0 );
	g_immediateContext->CSSetShaderResources ( 0, 1, &srv );
	g_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );
	g_immediateContext->CSSetConstantBuffers ( 0, 1, &constantBuffer );
	g_immediateContext->CSSetSamplers ( 0, 1, &samplerState );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

EASELERR EASEL_DLL esInitializeD3D11 ( ID3D11Device * d3dDevice, unsigned processingUnit )
{
	g_processingUnitD3D11 = processingUnit;
	if ( g_processingUnitD3D11 > 256 )
		return EASELERR_FAILED_INITIALIZE;

	if ( d3dDevice )
	{
		g_d3dDevice = d3dDevice;
		d3dDevice->AddRef ();
	}
	else
	{
		DWORD createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined ( DEBUG ) || defined ( _DEBUG )
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		if ( FAILED ( D3D11CreateDevice ( nullptr, D3D_DRIVER_TYPE_HARDWARE, 0,
			createFlags, nullptr, 0, D3D11_SDK_VERSION,
			&g_d3dDevice, nullptr, &g_immediateContext ) ) )
			return EASELERR_FAILED_INITIALIZE;
	}

	return EASELERR_SUCCEED;
}

#if defined ( DEBUG ) || defined ( _DEBUG )
#include <dxgi1_4.h>
#include <dxgidebug.h>
#pragma comment ( lib, "dxgi.lib" )
#pragma comment ( lib, "dxguid.lib" )
#endif

void EASEL_DLL esUninitializeD3D11 ()
{
	for ( auto i = g_uavsForTexture2D.begin (); i != g_uavsForTexture2D.end (); ++i )
		i->second->Release ();
	g_uavsForTexture2D.clear ();
	for ( auto i = g_srvsForTexture2D.begin (); i != g_srvsForTexture2D.end (); ++i )
		i->second->Release ();
	g_srvsForTexture2D.clear ();

	SAFE_RELEASE ( g_resizeSamplerLinear );
	SAFE_RELEASE ( g_resizeSamplerNearest );

	SAFE_RELEASE ( g_histogramUAV );
	SAFE_RELEASE ( g_histogramSRV );
	SAFE_RELEASE ( g_histogramBufferD3D11 );

	SAFE_RELEASE ( g_filteringConstantsD3D11 );

	for ( auto i = g_shadersD3D11.begin (); i != g_shadersD3D11.end (); ++i )
		i->second->Release ();
	g_shadersD3D11.clear ();

	SAFE_RELEASE ( g_immediateContext );
	SAFE_RELEASE ( g_d3dDevice );

#if defined ( DEBUG ) || defined ( _DEBUG )
	CComPtr<IDXGIDebug1> dxgiDebug;
	DXGIGetDebugInterface1 ( 0, __uuidof( IDXGIDebug1 ), ( void ** ) &dxgiDebug );
	dxgiDebug->ReportLiveObjects ( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL );
#endif
}

EASELERR EASEL_DLL esGetD3D11Device ( ID3D11Device ** result )
{
	if ( result == nullptr )
		return EASELERR_ARGUMENT_IS_NULL;
	if ( g_d3dDevice == nullptr )
		return EASELERR_DEVICE_IS_NULL;
	*result = g_d3dDevice;
	g_d3dDevice->AddRef ();
	return EASELERR_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

EASELERR EASEL_DLL esLoadTexture2DFromStream ( ID3D11Device * d3dDevice,
	IStream * stream, DXGI_FORMAT format, ID3D11Texture2D ** result )
{
	CComPtr<IWICImagingFactory> imagingFactory;
	if ( FAILED ( CoCreateInstance ( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, ( LPVOID* ) &imagingFactory ) ) )
		return EASELERR_FAILED_INITIALIZE;

	CComPtr<IWICBitmapDecoder> bitmapDecoder;
	if ( FAILED ( imagingFactory->CreateDecoderFromStream ( stream, nullptr,
		WICDecodeMetadataCacheOnDemand, &bitmapDecoder ) ) )
		return EASELERR_FAILED_INITIALIZE;

	CComPtr<IWICBitmapFrameDecode> bitmapFrame;
	if ( FAILED ( bitmapDecoder->GetFrame ( 0, &bitmapFrame ) ) )
		return EASELERR_FAILED_INITIALIZE;

	CComPtr<IWICFormatConverter> formatConverter;
	if ( FAILED ( imagingFactory->CreateFormatConverter ( &formatConverter ) ) )
		return EASELERR_FAILED_INITIALIZE;
	if ( FAILED ( formatConverter->Initialize ( bitmapFrame,
		__esConvertPixelFormatGUID ( format ),
		WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom ) ) )
		return EASELERR_FAILED_INITIALIZE;

	D3D11_TEXTURE2D_DESC texDesc = { 0, };
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	bitmapFrame->GetSize ( &texDesc.Width, &texDesc.Height );
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	byte * copyMemory = new byte [ texDesc.Width * texDesc.Height * 4 ];
	if ( FAILED ( formatConverter->CopyPixels ( nullptr,
		texDesc.Width * 4,
		texDesc.Width * texDesc.Height * 4,
		copyMemory ) ) )
	{
		delete [] copyMemory;
		return EASELERR_FAILED_INITIALIZE;
	}
	
	if ( d3dDevice == nullptr )
		d3dDevice = g_d3dDevice;

	D3D11_SUBRESOURCE_DATA initialData = { 0, };
	initialData.pSysMem = copyMemory;
	initialData.SysMemPitch = texDesc.Width * 4;
	initialData.SysMemSlicePitch = texDesc.Width * 4 * texDesc.Height;
	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, &initialData, result ) ) )
	{
		delete [] copyMemory;
		return EASELERR_FAILED_INITIALIZE;
	}

	delete [] copyMemory;

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esLoadTexture2DFromFile ( ID3D11Device * d3dDevice,
	LPCTSTR filename, DXGI_FORMAT format, ID3D11Texture2D ** result )
{
	CComPtr<IStream> stream;
	HRESULT hr;
	if ( FAILED ( hr = SHCreateStreamOnFile ( filename, STGM_READ, &stream ) ) )
		return EASELERR_FILE_IS_NOT_FOUND;
	return esLoadTexture2DFromStream ( d3dDevice, stream, format, result );
}

EASELERR EASEL_DLL esCreateCompatibleTexture2D ( ID3D11Device * d3dDevice, ID3D11Texture2D * original, ID3D11Texture2D ** result )
{
	if ( d3dDevice == nullptr )
		d3dDevice = g_d3dDevice;

	D3D11_TEXTURE2D_DESC texDesc;
	original->GetDesc ( &texDesc );

	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, nullptr, result ) ) )
		return EASELERR_FAILED_INITIALIZE;

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esCreateCompatibleScaleTexture2D ( ID3D11Device * d3dDevice, ID3D11Texture2D * original, int scale, ID3D11Texture2D ** result )
{
	if ( d3dDevice == nullptr )
		d3dDevice = g_d3dDevice;

	D3D11_TEXTURE2D_DESC texDesc;
	original->GetDesc ( &texDesc );
	texDesc.Width = ( UINT ) ( texDesc.Width * ( scale / 100.0f ) );
	texDesc.Height = ( UINT ) ( texDesc.Height * ( scale / 100.0f ) );

	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, nullptr, result ) ) )
		return EASELERR_FAILED_INITIALIZE;

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esSaveTexture2DToStream ( ID3D11Device * d3dDevice, IStream * stream, ID3D11Texture2D * target )
{
	CComPtr<IWICImagingFactory> imagingFactory;
	if ( FAILED ( CoCreateInstance ( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, ( LPVOID* ) &imagingFactory ) ) )
		return EASELERR_FAILED_INITIALIZE;

	CComPtr<IWICBitmapEncoder> bitmapEncoder;
	if ( FAILED ( imagingFactory->CreateEncoder ( GUID_ContainerFormatPng,
		nullptr, &bitmapEncoder ) ) )
		return EASELERR_FAILED_INITIALIZE;

	if ( FAILED ( bitmapEncoder->Initialize ( stream, WICBitmapEncoderNoCache ) ) )
		return EASELERR_FAILED_INITIALIZE;

	CComPtr<IWICBitmapFrameEncode> frameEncode;
	CComPtr<IPropertyBag2> encoderOptions;
	if ( FAILED ( bitmapEncoder->CreateNewFrame ( &frameEncode, &encoderOptions ) ) )
		return EASELERR_FAILED_INITIALIZE;
	if ( FAILED ( frameEncode->Initialize ( nullptr ) ) )
		return EASELERR_FAILED_INITIALIZE;

	D3D11_TEXTURE2D_DESC texDesc;
	target->GetDesc ( &texDesc );

	if ( FAILED ( frameEncode->SetResolution ( 96, 96 ) ) )
		return EASELERR_FAILED_INITIALIZE;
	if ( FAILED ( frameEncode->SetSize ( texDesc.Width, texDesc.Height ) ) )
		return EASELERR_FAILED_INITIALIZE;
	GUID encodeGuid = __esConvertPixelFormatGUID ( texDesc.Format );
	if ( FAILED ( frameEncode->SetPixelFormat ( &encodeGuid ) ) )
		return EASELERR_FAILED_INITIALIZE;

	if ( d3dDevice == nullptr )
		d3dDevice = g_d3dDevice;

	texDesc.BindFlags = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	CComPtr<ID3D11Texture2D> lockTexture;
	if ( FAILED ( d3dDevice->CreateTexture2D ( &texDesc, nullptr, &lockTexture ) ) )
		return EASELERR_FAILED_INITIALIZE;
	CComPtr<ID3D11DeviceContext> immediateContext;
	d3dDevice->GetImmediateContext ( &immediateContext );
	immediateContext->CopyResource ( lockTexture, target );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if ( FAILED ( immediateContext->Map ( lockTexture, 0, D3D11_MAP_READ, 0, &mappedResource ) ) )
		return EASELERR_FAILED_INITIALIZE;
	if ( FAILED ( frameEncode->WritePixels ( texDesc.Height,
		texDesc.Width * 4,
		texDesc.Width * texDesc.Height * 4,
		( BYTE* ) mappedResource.pData ) ) )
		return EASELERR_FAILED_INITIALIZE;
	immediateContext->Unmap ( lockTexture, 0 );

	if ( FAILED ( frameEncode->Commit () ) )
		return EASELERR_FAILED_INITIALIZE;
	if ( FAILED ( bitmapEncoder->Commit () ) )
		return EASELERR_FAILED_INITIALIZE;

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esSaveTexture2DToFile ( ID3D11Device * d3dDevice, LPCTSTR filename, ID3D11Texture2D * target )
{
	CComPtr<IStream> stream;
	if ( FAILED ( SHCreateStreamOnFile ( filename, STGM_WRITE | STGM_CREATE, &stream ) ) )
		return EASELERR_FILE_IS_NOT_FOUND;
	return esSaveTexture2DToStream ( d3dDevice, stream, target );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

EASELERR EASEL_DLL esDoFiltering ( ID3D11Texture2D * destination, ID3D11Texture2D * source, const EASEL_FILTER & filter )
{
	__egResetComputeShaderInContextD3D11 ();

	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	destination->GetDesc ( &destDesc );
	source->GetDesc ( &srcDesc );

	if ( ( destDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS ) == 0
		|| ( srcDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE ) == 0 )
		return EASELERR_UNAUTHORIZED_OBJECT;
	if ( destDesc.Width != srcDesc.Width || destDesc.Height != srcDesc.Height )
		return EASELERR_INVALID_OBJECT;

	EGMACRO macro ( g_processingUnitD3D11, srcDesc.Format, destDesc.Format, "Filtering" );
	ID3D11ComputeShader * computeShader = __egGetShader ( &macro );
	if ( computeShader == nullptr ) return EASELERR_SHADER_COMPILE_ERROR;

	ID3D11ShaderResourceView * srv = __egGetShaderResourceView ( source );
	if ( srv == nullptr ) return EASELERR_INVALID_OBJECT;
	
	ID3D11UnorderedAccessView * uav = __egGetUnorderedAccessView ( destination );
	if ( uav == nullptr ) return EASELERR_INVALID_OBJECT;

	if ( g_filteringConstantsD3D11 == nullptr )
	{
		D3D11_BUFFER_DESC constantBufferDesc = { 0, };
		constantBufferDesc.ByteWidth = sizeof ( EASEL_FILTER );
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if ( FAILED ( g_d3dDevice->CreateBuffer ( &constantBufferDesc, nullptr, &g_filteringConstantsD3D11 ) ) )
			return EASELERR_FAILED_INITIALIZE;
	}
	g_immediateContext->UpdateSubresource ( g_filteringConstantsD3D11, 0, nullptr, &filter, sizeof ( EASEL_FILTER ), 0 );

	ID3D11Buffer * constantBuffer = g_filteringConstantsD3D11;

	g_immediateContext->CSSetShader ( computeShader, nullptr, 0 );
	g_immediateContext->CSSetShaderResources ( 0, 1, &srv );
	g_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );
	g_immediateContext->CSSetConstantBuffers ( 0, 1, &constantBuffer );
	g_immediateContext->Dispatch (
		( UINT ) ceill ( ( float ) destDesc.Width / g_processingUnitD3D11 ),
		( UINT ) ceill ( ( float ) destDesc.Height / g_processingUnitD3D11 ),
		1 );

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esDoCopyResource ( ID3D11Texture2D * destination, ID3D11Texture2D * source )
{
	__egResetComputeShaderInContextD3D11 ();

	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	destination->GetDesc ( &destDesc );
	source->GetDesc ( &srcDesc );

	if ( ( destDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS ) == 0
		|| ( srcDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE ) == 0 )
		return EASELERR_UNAUTHORIZED_OBJECT;
	if ( destDesc.Width != srcDesc.Width || destDesc.Height != srcDesc.Height )
		return EASELERR_INVALID_OBJECT;

	if ( destDesc.Format == srcDesc.Format )
		g_immediateContext->CopyResource ( destination, source );
	else
	{
		int target = 0;
		if ( destDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || destDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM )
			target = 1;
		else if ( destDesc.Format == DXGI_FORMAT_R32G32B32A32_FLOAT )
			target = 2;
		else return EASELERR_UNSUPPORT_FORMAT;

		EGMACRO macro ( g_processingUnitD3D11, srcDesc.Format, destDesc.Format, "Copy" );
		ID3D11ComputeShader * computeShader = __egGetShader ( &macro );
		if ( computeShader == nullptr ) return EASELERR_SHADER_COMPILE_ERROR;

		ID3D11ShaderResourceView * srv = __egGetShaderResourceView ( source );
		if ( srv == nullptr ) return EASELERR_INVALID_OBJECT;

		ID3D11UnorderedAccessView * uav = __egGetUnorderedAccessView ( destination );
		if ( uav == nullptr ) return EASELERR_INVALID_OBJECT;

		g_immediateContext->CSSetShader ( computeShader, nullptr, 0 );
		g_immediateContext->CSSetShaderResources ( 0, 1, &srv );
		g_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );
		g_immediateContext->Dispatch (
			( UINT ) ceill ( ( float ) destDesc.Width / g_processingUnitD3D11 ),
			( UINT ) ceill ( ( float ) destDesc.Height / g_processingUnitD3D11 ),
			1 );
	}

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esDoResize ( ID3D11Texture2D * destination, ID3D11Texture2D * source, EASEL_SAMPLING sampling )
{
	__egResetComputeShaderInContextD3D11 ();

	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	destination->GetDesc ( &destDesc );
	source->GetDesc ( &srcDesc );

	if ( ( destDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS ) == 0
		|| ( srcDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE ) == 0 )
		return EASELERR_UNAUTHORIZED_OBJECT;
	if ( destDesc.Width == srcDesc.Width && destDesc.Height == srcDesc.Height )
		return EASELERR_INVALID_OBJECT;

	EGMACRO macro ( g_processingUnitD3D11, srcDesc.Format, destDesc.Format, "Resize" );
	ID3D11ComputeShader * computeShader = __egGetShader ( &macro );
	if ( computeShader == nullptr ) return EASELERR_SHADER_COMPILE_ERROR;

	ID3D11ShaderResourceView * srv = __egGetShaderResourceView ( source );
	if ( srv == nullptr ) return EASELERR_INVALID_OBJECT;

	ID3D11UnorderedAccessView * uav = __egGetUnorderedAccessView ( destination );
	if ( uav == nullptr ) return EASELERR_INVALID_OBJECT;

	ID3D11SamplerState * samplerState = __egGetSamplerState ( sampling );
	if ( samplerState == nullptr ) return EASELERR_INVALID_OBJECT;

	g_immediateContext->CSSetShader ( computeShader, nullptr, 0 );
	g_immediateContext->CSSetShaderResources ( 0, 1, &srv );
	g_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );
	g_immediateContext->CSSetSamplers ( 0, 1, &samplerState );
	g_immediateContext->Dispatch (
		( UINT ) ceill ( ( float ) destDesc.Width / g_processingUnitD3D11 ),
		( UINT ) ceill ( ( float ) destDesc.Height / g_processingUnitD3D11 ),
		1 );

	return EASELERR_SUCCEED;
}

EASELERR EASEL_DLL esDoHistogramEqualization ( ID3D11Texture2D * destination, ID3D11Texture2D * source, const int histogram [ 256 ] )
{
	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	destination->GetDesc ( &destDesc );
	source->GetDesc ( &srcDesc );

	if ( g_histogramBufferD3D11 == nullptr )
	{
		D3D11_BUFFER_DESC constantBufferDesc = { 0, };
		constantBufferDesc.StructureByteStride = sizeof ( int );
		constantBufferDesc.ByteWidth = sizeof ( int ) * 256;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		if ( FAILED ( g_d3dDevice->CreateBuffer ( &constantBufferDesc, nullptr, &g_histogramBufferD3D11 ) ) )
			return EASELERR_FAILED_INITIALIZE;

		if ( FAILED ( g_d3dDevice->CreateShaderResourceView ( g_histogramBufferD3D11, nullptr, &g_histogramSRV ) ) )
			return EASELERR_FAILED_INITIALIZE;

		if ( FAILED ( g_d3dDevice->CreateUnorderedAccessView ( g_histogramBufferD3D11, nullptr, &g_histogramUAV ) ) )
			return EASELERR_FAILED_INITIALIZE;
	}

	int histogram_custom [ 256 ];
	if ( histogram == nullptr )
	{
		EGMACRO macro ( g_processingUnitD3D11, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, "GetHistogram" );
		ID3D11ComputeShader * computeShader = __egGetShader ( &macro );
		if ( computeShader == nullptr ) return EASELERR_SHADER_COMPILE_ERROR;

		g_immediateContext->CSSetShader ( computeShader, nullptr, 0 );
		ID3D11ShaderResourceView * srv = __egGetShaderResourceView ( source );
		if ( srv == nullptr ) return EASELERR_INVALID_OBJECT;
		ID3D11UnorderedAccessView * uav = g_histogramUAV;
		g_immediateContext->CSSetUnorderedAccessViews ( 1, 0, &uav, nullptr );
		g_immediateContext->Dispatch (
			( UINT ) ceill ( ( float ) srcDesc.Width / g_processingUnitD3D11 ),
			( UINT ) ceill ( ( float ) srcDesc.Height / g_processingUnitD3D11 ),
			1 );

		D3D11_MAPPED_SUBRESOURCE subres;
		g_immediateContext->Map ( g_histogramBufferD3D11, 0, D3D11_MAP_READ, 0, &subres );
		// TODO
		g_immediateContext->Unmap ( g_histogramBufferD3D11, 0 );

		histogram = histogram_custom;
	}

	// TODO

	return EASELERR_SUCCEED;
}

#endif