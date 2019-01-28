#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11GammaProcessor::D3D11GammaProcessor ( D3D11ComputeShaderProcessor * processor )
	: _processor ( processor )
{
	SetGammaValue ();
}

HRESULT __stdcall easel::d3d11::D3D11GammaProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::BitmapProcessor )
		|| riid == __uuidof ( easel::ComputeShaderBitmapProcessor ) || riid == __uuidof ( easel::GammaSpaceBitmapProcessor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11GammaProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11GammaProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11GammaProcessor::Process ( Bitmap * destination, Bitmap * source )
{
	return _processor->Process ( destination, source );
}

HRESULT easel::d3d11::D3D11GammaProcessor::GetFactory ( EaselFactory ** factory )
{
	return _processor->GetFactory ( factory );
}

BOOL easel::d3d11::D3D11GammaProcessor::CheckBitmapSize ( Bitmap * b1, Bitmap * b2 )
{
	return _processor->CheckBitmapSize ( b1, b2 );
}

void easel::d3d11::D3D11GammaProcessor::SetConstantBufferData ( const void * data )
{
	_processor->SetConstantBufferData ( data );
}

void easel::d3d11::D3D11GammaProcessor::SetAdditionalState ()
{
}

void easel::d3d11::D3D11GammaProcessor::SetGammaValue ( float gamma, bool toGammaSpace )
{
	GammaSpaceArgs args = { toGammaSpace ? args.gamma = gamma : 1 / gamma, 0, 0, 0 };
	SetConstantBufferData ( &args );
}
