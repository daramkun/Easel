#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11ComputeShaderProcessor::D3D11ComputeShaderProcessor ( D3D11EaselFactory * factory, ID3D11ComputeShader * computeShader, ID3D11Buffer * constantBuffer, UINT constantBufferSize )
	: _refCount ( 1 ), _factory ( factory )
	, _shader ( computeShader ), _constantBuffer ( constantBuffer ), _constantBufferSize ( constantBufferSize )
{

}

HRESULT __stdcall easel::d3d11::D3D11ComputeShaderProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::BitmapProcessor )
		|| riid == __uuidof ( easel::ComputeShaderBitmapProcessor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11ComputeShaderProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11ComputeShaderProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11ComputeShaderProcessor::Process ( easel::Bitmap * destination, easel::Bitmap * source )
{
	if ( destination == nullptr || source == nullptr )
		return FALSE;

	D3D11Bitmap * b1 = static_cast< D3D11Bitmap* > ( destination ),
		*b2 = static_cast< D3D11Bitmap* >( source );

	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	b1->_texture->GetDesc ( &destDesc );
	b2->_texture->GetDesc ( &srcDesc );

	if ( ( destDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS ) == 0
		|| ( srcDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE ) == 0 )
		return FALSE;

	if ( !CheckBitmapSize ( destination, source ) )
		return FALSE;

	ID3D11UnorderedAccessView * uav = b1->_uav;
	ID3D11ShaderResourceView * srv = b2->_srv;

	_factory->_immediateContext->ClearState ();

	_factory->_immediateContext->CSSetShader ( _shader, nullptr, 0 );
	_factory->_immediateContext->CSSetUnorderedAccessViews ( 0, 1, &uav, nullptr );
	_factory->_immediateContext->CSSetShaderResources ( 0, 1, &srv );
	SetAdditionalState ();

	if ( _constantBuffer != nullptr )
	{
		ID3D11Buffer * buffer = _constantBuffer;
		_factory->_immediateContext->CSSetConstantBuffers ( 0, 1, &buffer );
	}

	_factory->_immediateContext->Dispatch (
		( UINT ) ceill ( ( float ) destDesc.Width / 16 ),
		( UINT ) ceill ( ( float ) destDesc.Height / 16 ),
		1 );

	return TRUE;
}

BOOL easel::d3d11::D3D11ComputeShaderProcessor::CheckBitmapSize ( easel::Bitmap * b1, easel::Bitmap * b2 )
{
	return b1->Width () == b2->Width () && b1->Height () == b2->Height ();
}

void easel::d3d11::D3D11ComputeShaderProcessor::SetConstantBufferData ( const void * data )
{
	if ( _constantBuffer == nullptr ) return;
	_factory->_immediateContext->UpdateSubresource ( _constantBuffer, 0, nullptr, data, _constantBufferSize, 0 );
}

void easel::d3d11::D3D11ComputeShaderProcessor::SetAdditionalState () { }

HRESULT easel::d3d11::D3D11ComputeShaderProcessor::GetFactory ( EaselFactory ** factory )
{
	*factory = _factory;
	( *factory )->AddRef ();
	return S_OK;
}