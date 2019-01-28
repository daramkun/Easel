#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11HistogramProcessor::D3D11HistogramProcessor ( D3D11ComputeShaderProcessor * processor )
	: _refCount ( 1 ), _processor ( processor )
{
}

HRESULT __stdcall easel::d3d11::D3D11HistogramProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::FilterBitmapProcessor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11HistogramProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11HistogramProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11HistogramProcessor::Process ( Bitmap * destination, Bitmap * source )
{
	return _processor->Process ( destination, source );
}

HRESULT easel::d3d11::D3D11HistogramProcessor::GetFactory ( EaselFactory ** factory )
{
	return _processor->GetFactory ( factory );
}

BOOL easel::d3d11::D3D11HistogramProcessor::CheckBitmapSize ( Bitmap * b1, Bitmap * b2 )
{
	return _processor->CheckBitmapSize ( b1, b2 );
}

void easel::d3d11::D3D11HistogramProcessor::SetConstantBufferData ( const void * data )
{
	_processor->SetConstantBufferData ( data );
}

void easel::d3d11::D3D11HistogramProcessor::SetAdditionalState ()
{
}

void easel::d3d11::D3D11HistogramProcessor::SetHistogram ( int histogram [ 256 ] )
{
	SetConstantBufferData ( histogram );
}
