#include "../Easel.h"
#include "Easel.D3D11.Internal.h"

easel::d3d11::D3D11ThresholdHistogramGenerator::D3D11ThresholdHistogramGenerator ( D3D11EaselFactory * factory )
	: _refCount ( 1 ), _factory ( factory )
	, _thresholdCount ( 0 )
{
}

HRESULT __stdcall easel::d3d11::D3D11ThresholdHistogramGenerator::QueryInterface ( REFIID riid, void ** ppvObject )
{
	if ( riid == __uuidof ( IUnknown )
		|| riid == __uuidof ( easel::HistogramGenerator ) || riid == __uuidof ( easel::ThresholdHistogramGenerator ) )
	{
		*ppvObject = this;
		AddRef ();
		return S_OK;
	}
	return E_FAIL;
}

ULONG __stdcall easel::d3d11::D3D11ThresholdHistogramGenerator::AddRef ( void )
{
	return InterlockedIncrement ( &_refCount );
}

ULONG __stdcall easel::d3d11::D3D11ThresholdHistogramGenerator::Release ( void )
{
	auto ret = InterlockedDecrement ( &_refCount );
	if ( ret == 0 )
		delete this;
	return ret;
}

BOOL easel::d3d11::D3D11ThresholdHistogramGenerator::Generate ( int buffer [ 256 ] )
{
	if ( _thresholdCount == 0 )
		return FALSE;

	Threshold * start = _thresholds, *end = _thresholds + _thresholdCount;
	int index = 0, weight = start->thresholdWeight;
	while ( start != end )
	{
		if ( weight <= index )
		{
			++start;
			weight += start->thresholdWeight;
		}

		buffer [ index++ ] = start->thresholdY;

		if ( index >= 256 )
			break;
	}

	return TRUE;
}

HRESULT easel::d3d11::D3D11ThresholdHistogramGenerator::GetFactory ( EaselFactory ** factory )
{
	*factory = _factory;
	( *factory )->AddRef ();
	return S_OK;
}

void easel::d3d11::D3D11ThresholdHistogramGenerator::SetThreshold ( Threshold * thresholds, int thresholdCount )
{
	if ( thresholdCount > 255 ) return;
	if ( thresholdCount == 0 )
	{
		_thresholdCount = 0;
		return;
	}

	memcpy ( _thresholds, thresholds, sizeof ( Threshold ) * ( _thresholdCount = thresholdCount ) );

	std::qsort ( _thresholds, _thresholdCount, sizeof ( int ), [] ( void const * first, void const * second )
	{
		if ( *( int* ) first > *( int* ) second ) return 1;
		else if ( *( int* ) first < *( int* ) second ) return -1;
		else return 0;
	} );
}
