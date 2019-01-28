#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11ResizeProcessor::D3D11ResizeProcessor ( D3D11ComputeShaderProcessor * processor )
	: _refCount ( 1 ), _processor ( processor )
{ SetSamplingFilter ( easel::kSamplingFilter_Nearest ); }

HRESULT __stdcall easel::d3d11::D3D11ResizeProcessor::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown ) || riid == __uuidof ( easel::BitmapProcessor )
		|| riid == __uuidof ( easel::ComputeShaderBitmapProcessor ) || riid == __uuidof ( easel::ResizeBitmapProcessor ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11ResizeProcessor::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11ResizeProcessor::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11ResizeProcessor::Process ( Bitmap * destination, Bitmap * source )
{
	return _processor->Process ( destination, source );
}

HRESULT easel::d3d11::D3D11ResizeProcessor::GetFactory ( EaselFactory ** factory )
{
	return _processor->GetFactory ( factory );
}

BOOL easel::d3d11::D3D11ResizeProcessor::CheckBitmapSize ( Bitmap * b1, Bitmap * b2 )
{
	return _processor->CheckBitmapSize ( b1, b2 );
}

void easel::d3d11::D3D11ResizeProcessor::SetConstantBufferData ( const void * data )
{
	_processor->SetConstantBufferData ( data );
}

void easel::d3d11::D3D11ResizeProcessor::SetAdditionalState ()
{
	CComPtr<D3D11EaselFactory> factory;
	GetFactory ( ( easel::EaselFactory ** ) &factory );

	ID3D11SamplerState * samplerState = _samplerState;
	factory->_immediateContext->CSSetSamplers ( 0, 1, &samplerState );
}

void easel::d3d11::D3D11ResizeProcessor::SetSamplingFilter ( SamplingFilter filter )
{
	CComPtr<D3D11EaselFactory> factory;
	GetFactory ( ( easel::EaselFactory ** ) &factory );

	if ( _samplerState )
		_samplerState.Release ();

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	switch ( filter )
	{
		case easel::kSamplingFilter_Nearest: samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case easel::kSamplingFilter_Linear: samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
	}
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxAnisotropy = 0;

	factory->_d3dDevice->CreateSamplerState ( &samplerDesc, &_samplerState );
}
