#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11HistogramExtractor::D3D11HistogramExtractor ( D3D11EaselFactory * factory, ID3D11ComputeShader * shader,
	ID3D11Buffer * buffer, ID3D11UnorderedAccessView * bufferUAV, ID3D11Buffer * copyOnly )
	: _refCount ( 1 ), _factory ( factory )
	, _shader ( shader ), _buffer ( buffer ), _bufferUAV ( bufferUAV ), _copyOnly ( copyOnly )
{
}

HRESULT __stdcall easel::d3d11::D3D11HistogramExtractor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown )
		|| riid == __uuidof ( easel::HistogramGenerator ) || riid == __uuidof ( easel::BitmapHistogramExtractor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11HistogramExtractor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11HistogramExtractor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11HistogramExtractor::Generate ( int buffer [ 256 ] )
{
	CComPtr<D3D11EaselFactory> factory;
	GetFactory ( ( easel::EaselFactory ** ) &factory );

	D3D11_TEXTURE2D_DESC srcDesc;
	_bitmap->_texture->GetDesc ( &srcDesc );

	ID3D11ShaderResourceView * srv = _bitmap->_srv;
	ID3D11UnorderedAccessView * uav = _bufferUAV;

	factory->_immediateContext->CSSetShader ( _shader, nullptr, 0 );
	factory->_immediateContext->CSSetShaderResources ( 0, 1, &srv );
	factory->_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );

	factory->_immediateContext->Dispatch (
		( UINT ) ceill ( ( float ) srcDesc.Width / 16 ),
		( UINT ) ceill ( ( float ) srcDesc.Height / 16 ),
		1 );

	factory->_immediateContext->CopyResource ( _copyOnly, _buffer );

	D3D11_MAPPED_SUBRESOURCE subres;
	factory->_immediateContext->Map ( _copyOnly, 0, D3D11_MAP_READ, 0, &subres );
	memcpy ( buffer, subres.pData, sizeof ( int ) * 256 );
	factory->_immediateContext->Unmap ( _copyOnly, 0 );

	if ( _autoCorrection )
	{
		float aspect = 255.0f / ( srcDesc.Width * srcDesc.Height );
		int equalization [ 256 ] = { 0, };
		for ( int i = 0, sum = 0; i < 256; ++i )
		{
			sum += buffer [ i ];
			equalization [ i ] = ( int ) round ( sum * aspect );
		}

		memcpy ( buffer, equalization, sizeof ( int ) * 256 );
	}

	return TRUE;
}

HRESULT easel::d3d11::D3D11HistogramExtractor::GetFactory ( EaselFactory ** factory )
{
	*factory = _factory;
	( *factory )->AddRef ();
	return S_OK;
}

void easel::d3d11::D3D11HistogramExtractor::SetBitmap ( Bitmap * bitmap, bool autoCorrection )
{
	_bitmap = static_cast< D3D11Bitmap* >( bitmap );
	_autoCorrection = autoCorrection;
}
