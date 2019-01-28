#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

#include <vector>
#include <atlconv.h>

#pragma comment ( lib, "windowscodecs.lib" )
#pragma comment ( lib, "dxgi.lib" )
#pragma comment ( lib, "d3d11.lib" )
#pragma comment ( lib, "d3dcompiler.lib" )

bool __LoadShaderData ( LPCWSTR filename, char ** shaderData, size_t * length )
{
	USES_CONVERSION;

	FILE * fp;
	fopen_s ( &fp, W2A ( filename ), "rb" );
	if ( fp == nullptr )
		return false;

	fseek ( fp, 0, SEEK_END );
	*length = ftell ( fp );
	fseek ( fp, 0, SEEK_SET );

	*shaderData = new char [ *length ];

	size_t readed = fread ( *shaderData, 1, *length, fp );

	fclose ( fp );

	return readed == *length;
}

easel::d3d11::D3D11EaselFactory::D3D11EaselFactory ( IWICImagingFactory * wicImagingFactory, ID3D11Device * d3dDevice, ID3D11DeviceContext * immediateContext )
	: _refCount ( 1 )
	, _wicImagingFactory ( wicImagingFactory )
	, _d3dDevice ( d3dDevice ), _immediateContext ( immediateContext )
{ }

HRESULT __stdcall easel::d3d11::D3D11EaselFactory::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::EaselFactory ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11EaselFactory::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11EaselFactory::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateBitmap ( UINT width, UINT height, easel::Bitmap ** bitmap )
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.ArraySize = 1;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	CComPtr<ID3D11Texture2D> texture;
	if ( FAILED ( _d3dDevice->CreateTexture2D ( &texDesc, nullptr, &texture ) ) )
		return E_FAIL;

	CComPtr<ID3D11ShaderResourceView> srv;
	if ( FAILED ( _d3dDevice->CreateShaderResourceView ( texture, nullptr, &srv ) ) )
		return E_FAIL;
	CComPtr<ID3D11UnorderedAccessView> uav;
	if ( FAILED ( _d3dDevice->CreateUnorderedAccessView ( texture, nullptr, &uav ) ) )
		return E_FAIL;
	CComPtr<ID3D11RenderTargetView> rtv;
	if ( FAILED ( _d3dDevice->CreateRenderTargetView ( texture, nullptr, &rtv ) ) )
		return E_FAIL;

	*bitmap = new D3D11Bitmap ( this, texture, srv, uav, rtv );
	if ( *bitmap == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateBitmapFromFile ( LPCWSTR filename, easel::Bitmap ** bitmap )
{
	CComPtr<IWICBitmapDecoder> decoder;
	if ( FAILED ( _wicImagingFactory->CreateDecoderFromFilename ( filename, nullptr,
		GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder ) ) )
		throw std::runtime_error ( "Failed create Decoder." );

	CComPtr<IWICBitmapFrameDecode> decoded;
	if ( FAILED ( decoder->GetFrame ( 0, &decoded ) ) )
		throw std::runtime_error ( "Failed get frame from image." );

	CComPtr<IWICFormatConverter> formatConverter;
	_wicImagingFactory->CreateFormatConverter ( &formatConverter );

	if ( FAILED ( formatConverter->Initialize ( decoded, GUID_WICPixelFormat128bppRGBFloat,
		WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom ) ) )
		throw std::runtime_error ( "Unsupported Pixel format." );

	UINT width, height;
	formatConverter->GetSize ( &width, &height );

	std::vector<BYTE> buffer ( width * 16 * height );
	if ( FAILED ( formatConverter->CopyPixels ( nullptr, width * 16, width * 16 * height, buffer.data () ) ) )
		throw std::runtime_error ( "Failed copy pixels from Windows Imaging Codec object." );

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.ArraySize = 1;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = buffer.data ();
	initialData.SysMemPitch = width * 16;
	initialData.SysMemSlicePitch = width * 16 * height;

	CComPtr<ID3D11Texture2D> texture;
	if ( FAILED ( _d3dDevice->CreateTexture2D ( &texDesc, &initialData, &texture ) ) )
		return E_FAIL;

	CComPtr<ID3D11ShaderResourceView> srv;
	if ( FAILED ( _d3dDevice->CreateShaderResourceView ( texture, nullptr, &srv ) ) )
		return E_FAIL;
	CComPtr<ID3D11UnorderedAccessView> uav;
	if ( FAILED ( _d3dDevice->CreateUnorderedAccessView ( texture, nullptr, &uav ) ) )
		return E_FAIL;
	CComPtr<ID3D11RenderTargetView> rtv;
	if ( FAILED ( _d3dDevice->CreateRenderTargetView ( texture, nullptr, &rtv ) ) )
		return E_FAIL;

	*bitmap = new D3D11Bitmap ( this, texture, srv, uav, rtv );
	if ( *bitmap == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CopyBitmap ( easel::Bitmap * destination, easel::Bitmap * source )
{
	if ( destination == nullptr || source == nullptr ) return E_FAIL;
	D3D11Bitmap * b1 = static_cast< D3D11Bitmap* >( destination );
	D3D11Bitmap * b2 = static_cast< D3D11Bitmap* >( source );
	_immediateContext->CopyResource ( b1->_texture, b2->_texture );
	return S_OK;
}

void easel::d3d11::D3D11EaselFactory::Swap ( easel::Bitmap * _b1, easel::Bitmap * _b2 )
{
	if ( _b1 == nullptr || _b2 == nullptr )
		return;

	D3D11Bitmap * b1 = static_cast< D3D11Bitmap* >( _b1 );
	D3D11Bitmap * b2 = static_cast< D3D11Bitmap* >( _b2 );

	auto tex = b1->_texture;
	auto srv = b1->_srv;
	auto uav = b1->_uav;
	auto rtv = b1->_rtv;

	b1->_texture = b2->_texture;
	b1->_srv = b2->_srv;
	b1->_uav = b2->_uav;
	b1->_rtv = b2->_rtv;

	b2->_texture = tex;
	b2->_srv = srv;
	b2->_uav = uav;
	b2->_rtv = rtv;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateComputeShaderProcessor ( LPCSTR shaderCode, bool useConstantBuffer, int constantBufferSize,
	easel::ComputeShaderBitmapProcessor ** processor )
{
	CComPtr<ID3DBlob> shaderBlob, errorMsg;
	if ( FAILED ( D3DCompile ( shaderCode, strlen ( shaderCode ), nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &shaderBlob, &errorMsg ) ) )
	{
		OutputDebugStringA ( ( const char * ) errorMsg->GetBufferPointer () );
		return E_FAIL;
	}

	CComPtr<ID3D11ComputeShader> computeShader;
	if ( FAILED ( _d3dDevice->CreateComputeShader ( shaderBlob->GetBufferPointer (), shaderBlob->GetBufferSize (), nullptr, &computeShader ) ) )
		return E_FAIL;

	CComPtr<ID3D11Buffer> constantBuffer;
	if ( useConstantBuffer )
	{
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = constantBufferSize;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		if ( FAILED ( _d3dDevice->CreateBuffer ( &constantBufferDesc, nullptr, &constantBuffer ) ) )
			return E_FAIL;
	}

	*processor = new D3D11ComputeShaderProcessor ( this, computeShader, constantBuffer, constantBufferSize );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateComputeShaderProcessor ( LPCWSTR filename, bool useConstantBuffer, int constantBufferSize,
	easel::ComputeShaderBitmapProcessor ** processor )
{
	char * shaderData;
	size_t dataLength;
	if ( __LoadShaderData ( filename, &shaderData, &dataLength ) == false )
		return E_FAIL;

	CComPtr<ID3D11ComputeShader> computeShader;
	if ( FAILED ( _d3dDevice->CreateComputeShader ( shaderData, dataLength, nullptr, &computeShader ) ) )
		return E_FAIL;

	delete [] shaderData;

	CComPtr<ID3D11Buffer> constantBuffer;
	if ( useConstantBuffer )
	{
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = constantBufferSize;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		if ( FAILED ( _d3dDevice->CreateBuffer ( &constantBufferDesc, nullptr, &constantBuffer ) ) )
			return E_FAIL;
	}

	*processor = new D3D11ComputeShaderProcessor ( this, computeShader, constantBuffer, constantBufferSize );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateSaturateProcessor ( easel::BitmapProcessor ** processor )
{
	return CreateComputeShaderProcessor ( L"SaturateShader.cso", false, 0,
		( easel::ComputeShaderBitmapProcessor ** ) processor );
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateRGB2YUVProcessor ( easel::BitmapProcessor ** processor )
{
	return CreateComputeShaderProcessor ( L"RGB2YUVConvertShader.cso", false, 0,
		( easel::ComputeShaderBitmapProcessor ** ) processor );
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateYUV2RGBProcessor ( easel::BitmapProcessor ** processor )
{
	return CreateComputeShaderProcessor ( L"YUV2RGBConvertShader.cso", false, 0,
		( easel::ComputeShaderBitmapProcessor ** ) processor );
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateRGB2GrayscaleProcessor ( easel::BitmapProcessor ** processor )
{
	return CreateComputeShaderProcessor ( L"RGB2GrayscaleConvertShader.cso", false, 0,
		( easel::ComputeShaderBitmapProcessor ** ) processor );
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateFilterProcessor ( easel::FilterBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"FilterShader.cso", true, sizeof ( Filter ), ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;
	
	*processor = new D3D11FilterProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateGammaSpaceProcessor ( easel::GammaSpaceBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"GammaSpaceShader.cso", true, sizeof ( GammaSpaceArgs ), ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;

	*processor = new D3D11GammaProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateRotationProcessor ( easel::RotationBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"RotationShader.cso", true, sizeof ( RotationArgs ), ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;

	*processor = new D3D11RotationProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateResizeProcessor ( easel::ResizeBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"ResizeShader.cso", false, 0, ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;

	*processor = new D3D11ResizeProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateArithmeticProcessor ( easel::ArithmeticBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"ArithmeticOperationShader.cso", true, sizeof ( ArithmeticOp ), ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;

	*processor = new D3D11ArithmeticProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateHistogramProcessor ( easel::HistogramBitmapProcessor ** processor )
{
	CComPtr<D3D11ComputeShaderProcessor> p;
	if ( FAILED ( CreateComputeShaderProcessor ( L"HistogramEqualizationShader.cso", true, sizeof ( int ) * 256, ( easel::ComputeShaderBitmapProcessor ** ) &p ) ) )
		return E_FAIL;

	*processor = new D3D11HistogramProcessor ( p );
	if ( *processor == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateHistogramExtractor ( easel::BitmapHistogramExtractor ** generator )
{
	char * shaderData;
	size_t dataLength;
	if ( __LoadShaderData ( TEXT ( "HistogramCalculationShader.cso" ), &shaderData, &dataLength ) == false )
		return E_FAIL;

	CComPtr<ID3D11ComputeShader> shader;
	if ( FAILED ( _d3dDevice->CreateComputeShader ( shaderData, dataLength, nullptr, &shader ) ) )
		return E_FAIL;

	delete [] shaderData;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.ByteWidth = sizeof ( int ) * 256;
	bufferDesc.StructureByteStride = sizeof ( int );
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	CComPtr<ID3D11Buffer> buffer;
	if ( FAILED ( _d3dDevice->CreateBuffer ( &bufferDesc, nullptr, &buffer ) ) )
		return E_FAIL;

	CComPtr<ID3D11UnorderedAccessView> uav;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	//uavDesc.Format = DXGI_FORMAT_R32_SINT;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 256;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	if ( FAILED ( _d3dDevice->CreateUnorderedAccessView ( buffer, &uavDesc, &uav ) ) )
		return E_FAIL;

	D3D11_BUFFER_DESC copyOnlyConstantBufferDesc = {};
	copyOnlyConstantBufferDesc.BindFlags = 0;
	copyOnlyConstantBufferDesc.ByteWidth = sizeof ( int ) * 256;
	copyOnlyConstantBufferDesc.Usage = D3D11_USAGE_STAGING;
	copyOnlyConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	CComPtr<ID3D11Buffer> copyOnly;
	if ( FAILED ( _d3dDevice->CreateBuffer ( &copyOnlyConstantBufferDesc, nullptr, &copyOnly ) ) )
		return E_FAIL;

	*generator = new D3D11HistogramExtractor ( this, shader, buffer, uav, copyOnly );
	if ( *generator == nullptr )
		return E_FAIL;

	return S_OK;
}

HRESULT easel::d3d11::D3D11EaselFactory::CreateThresholdGenerator ( easel::ThresholdHistogramGenerator ** generator )
{
	*generator = new D3D11ThresholdHistogramGenerator ( this );
	if ( *generator == nullptr )
		return E_FAIL;
	return S_OK;
}

void easel::d3d11::D3D11EaselFactory::Show ( easel::Bitmap * bitmap, LPCSTR title )
{
	_immediateContext->ClearState ();

	WNDCLASS wndClass = {
		0, DefWindowProc, 0, 0,
		GetModuleHandle ( nullptr ), 0, 0, 0, 0,
		L"EaselDebugWindow"
	};
	RegisterClass ( &wndClass );

	RECT rect = { 0, 0, ( LONG ) bitmap->Width (), ( LONG ) bitmap->Height () };
	AdjustWindowRect ( &rect, WS_CAPTION | WS_SYSMENU, FALSE );

	USES_CONVERSION;

	HWND hWnd = CreateWindow ( wndClass.lpszClassName, A2W ( title ),
		WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, 0,
		wndClass.hInstance, nullptr );

	CComPtr<IDXGIFactory> factory;
	CreateDXGIFactory ( __uuidof ( IDXGIFactory ), ( void ** ) &factory );

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = bitmap->Width ();
	swapChainDesc.BufferDesc.Height = bitmap->Height ();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;
	CComPtr<IDXGISwapChain> swapChain;
	factory->CreateSwapChain ( _d3dDevice, &swapChainDesc, &swapChain );

	CComPtr<ID3D11Resource> resource;
	swapChain->GetBuffer ( 0, __uuidof ( ID3D11Resource ), ( void ** ) &resource );

	CComPtr<ID3D11RenderTargetView> swapChainRtv;
	_d3dDevice->CreateRenderTargetView ( resource, nullptr, &swapChainRtv );

	ID3D11RenderTargetView * _rtv = swapChainRtv;
	_immediateContext->OMSetRenderTargets ( 1, &_rtv, nullptr );
	D3D11_VIEWPORT viewport = { 0, 0, ( float ) swapChainDesc.BufferDesc.Width, ( float ) swapChainDesc.BufferDesc.Height, 0, 1 };
	_immediateContext->RSSetViewports ( 1, &viewport );

	CComPtr<ID3DBlob> errMsg;

	const char * vs =
		"struct OUTPUT { float4 position : SV_Position; float2 texcoord : TEXCOORD; };"
		""
		"static OUTPUT outputs [] = {"
		"	{ float4 ( -1, -1, 0, 1 ), float2 ( 0, 1 ) },"
		"	{ float4 ( -1, +1, 0, 1 ), float2 ( 0, 0 ) },"
		"	{ float4 ( +1, -1, 0, 1 ), float2 ( 1, 1 ) },"
		"	{ float4 ( +1, +1, 0, 1 ), float2 ( 1, 0 ) },"
		"};"
		""
		"OUTPUT main ( uint vertexId : SV_VertexID )"
		"{"
		"	return outputs [ vertexId ];"
		"}"
		;
	CComPtr<ID3DBlob> vsCode;
	D3DCompile ( vs, strlen ( vs ), nullptr, nullptr, nullptr, "main", "vs_5_0",
		0, 0, &vsCode, &errMsg );

	const char * ps =
		"Texture2D tex : register ( t0 );"
		"SamplerState samplerState {"
		"	Filter = MIN_MAG_MIP_POINT;"
		"	AddressU = Wrap;"
		"	AddressV = Wrap;"
		"	AddressW = Wrap;"
		"};"
		"float4 main ( float4 position : SV_Position, float2 texcoord : TEXCOORD ) : SV_Target"
		"{"
		"	return tex.Sample ( samplerState, texcoord );"
		"}"
		;
	CComPtr<ID3DBlob> psCode;
	D3DCompile ( ps, strlen ( ps ), nullptr, nullptr, nullptr, "main", "ps_5_0",
		0, 0, &psCode, &errMsg );

	CComPtr<ID3D11VertexShader> vertexShader;
	CComPtr<ID3D11PixelShader> pixelShader;

	_d3dDevice->CreateVertexShader ( vsCode->GetBufferPointer (), vsCode->GetBufferSize (), nullptr, &vertexShader );
	_d3dDevice->CreatePixelShader ( psCode->GetBufferPointer (), psCode->GetBufferSize (), nullptr, &pixelShader );

	CComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	_d3dDevice->CreateSamplerState ( &samplerDesc, &samplerState );

	_immediateContext->IASetPrimitiveTopology ( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	_immediateContext->VSSetShader ( vertexShader, nullptr, 0 );
	_immediateContext->PSSetShader ( pixelShader, nullptr, 0 );
	ID3D11SamplerState * samplerStateNative = samplerState;
	_immediateContext->PSSetSamplers ( 0, 1, &samplerStateNative );
	ID3D11ShaderResourceView * srv = static_cast< D3D11Bitmap* >( bitmap )->_srv;
	_immediateContext->PSSetShaderResources ( 0, 1, &srv );

	ShowWindow ( hWnd, SW_SHOW );

	_immediateContext->Draw ( 4, 0 );
	swapChain->Present ( 0, 0 );

	MSG msg;
	while ( GetMessage ( &msg, hWnd, 0, 0 ) )
	{
		if ( msg.message == 0 ) break;
		if ( msg.message == WM_CLOSE )
			PostQuitMessage ( 0 );
		else if ( msg.message == WM_PAINT )
		{
			swapChain->Present ( 0, 0 );
		}

		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}

	CloseWindow ( hWnd );
	DestroyWindow ( hWnd );
	UnregisterClass ( wndClass.lpszClassName, wndClass.hInstance );
}

HRESULT easel::CreateEaselFactoryD3D11 ( EaselFactory ** factory )
{
	CComPtr<IWICImagingFactory> wicImagingFactory;
	CComPtr<ID3D11Device> d3dDevice;
	CComPtr<ID3D11DeviceContext> immediateContext;

	if ( FAILED ( CoCreateInstance ( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, ( LPVOID* ) &wicImagingFactory ) ) )
		return E_FAIL;

	DWORD createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined ( DEBUG ) || defined ( _DEBUG )
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	if ( FAILED ( D3D11CreateDevice ( nullptr, D3D_DRIVER_TYPE_HARDWARE, 0,
		createFlags, nullptr, 0, D3D11_SDK_VERSION,
		&d3dDevice, nullptr, &immediateContext ) ) )
		return E_FAIL;

	*factory = new easel::d3d11::D3D11EaselFactory ( wicImagingFactory, d3dDevice, immediateContext );
	return S_OK;
}