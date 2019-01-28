#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11FilterProcessor::D3D11FilterProcessor ( D3D11ComputeShaderProcessor * processor )
	: _refCount ( 1 ), _processor ( processor )
{ }

HRESULT __stdcall easel::d3d11::D3D11FilterProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::BitmapProcessor )
		|| riid == __uuidof ( easel::ComputeShaderBitmapProcessor ) || riid == __uuidof ( easel::FilterBitmapProcessor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11FilterProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11FilterProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11FilterProcessor::Process ( Bitmap * destination, Bitmap * source )
{
	return _processor->Process ( destination, source );
}

HRESULT easel::d3d11::D3D11FilterProcessor::GetFactory ( EaselFactory ** factory )
{
	return _processor->GetFactory ( factory );
}

BOOL easel::d3d11::D3D11FilterProcessor::CheckBitmapSize ( Bitmap * b1, Bitmap * b2 )
{
	return _processor->CheckBitmapSize ( b1, b2 );
}

void easel::d3d11::D3D11FilterProcessor::SetConstantBufferData ( const void * data )
{
	_processor->SetConstantBufferData ( data );
}

void easel::d3d11::D3D11FilterProcessor::SetAdditionalState ()
{
}

void easel::d3d11::D3D11FilterProcessor::SetFilter ( const Filter & filter )
{
	SetConstantBufferData ( &filter );
}
