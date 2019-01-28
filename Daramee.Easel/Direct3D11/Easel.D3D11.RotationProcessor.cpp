#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11RotationProcessor::D3D11RotationProcessor ( D3D11ComputeShaderProcessor * processor )
	: _refCount ( 1 ), _processor ( processor )
{ SetRotationAngle ( easel::kRotation_0 ); }

HRESULT __stdcall easel::d3d11::D3D11RotationProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
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

ULONG __stdcall easel::d3d11::D3D11RotationProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11RotationProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11RotationProcessor::Process ( Bitmap * destination, Bitmap * source )
{
	return _processor->Process ( destination, source );
}

HRESULT easel::d3d11::D3D11RotationProcessor::GetFactory ( EaselFactory ** factory )
{
	return _processor->GetFactory ( factory );
}

BOOL easel::d3d11::D3D11RotationProcessor::CheckBitmapSize ( Bitmap * b1, Bitmap * b2 )
{
	if ( rot == easel::kRotation_0 || rot == easel::kRotation_180 )
		return _processor->CheckBitmapSize ( b1, b2 );
	return ( b1->Width () == b2->Height () && b1->Height () == b2->Width () );
}

void easel::d3d11::D3D11RotationProcessor::SetConstantBufferData ( const void * data )
{
	_processor->SetConstantBufferData ( data );
}

void easel::d3d11::D3D11RotationProcessor::SetAdditionalState ()
{
}

void easel::d3d11::D3D11RotationProcessor::SetRotationAngle ( Rotation rot )
{
	RotationArgs args = { this->rot = rot, 0, 0, 0 };
	SetConstantBufferData ( &args );
}
