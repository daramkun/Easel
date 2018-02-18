#define EASEL_D3D11
#include <easel.h>

#include <atlconv.h>
#include <atlbase.h>

int DoProgram ()
{
	CComPtr<ID3D11Texture2D> sampleTexture, targetTexture, resizeTexture;
	if ( esLoadTexture2DFromFile ( nullptr, L"Sample.jpg", DXGI_FORMAT_B8G8R8A8_UNORM, &sampleTexture ) != EASELERR_SUCCEED )
		return -2;
	D3D11_TEXTURE2D_DESC desc;
	sampleTexture->GetDesc ( &desc );
	if ( esCreateCompatibleTexture2D ( nullptr, sampleTexture, &targetTexture ) != EASELERR_SUCCEED )
		return -3;
	if ( esCreateCompatibleScaleTexture2D ( nullptr, targetTexture, 75, &resizeTexture ) != EASELERR_SUCCEED )
		return -4;

	float filterValues [] = { 
		/*0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f,
		0.00002292f, 0.00078634f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f,
		0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f,
		0.00038771f, 0.01330373f, 0.11098164f, 0.22508352f, 0.11098164f, 0.01330373f, 0.00038771f,
		0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f,
		0.00002292f, 0.00078633f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f,
		0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f,*/
		0, -2, 0,
		-2, 11, -2,
		0, -2, 0
	};
	EASEL_FILTER filter/* ( filterValues,
		( int ) sqrt ( _countof ( filterValues ) ) / 2,
		( int ) sqrt ( _countof ( filterValues ) ) / 2 );
	filter.divide ( 3 )*/ = EASEL_FILTER::getSharpenFilter ();
	if ( esDoFiltering ( targetTexture, sampleTexture, filter ) != EASELERR_SUCCEED )
		return -5;

	if ( esDoResize ( resizeTexture, targetTexture, EASEL_SAMPLING_LINEAR ) )
		return -6;

	if ( esSaveTexture2DToFile ( nullptr, L"Result.png", resizeTexture ) != EASELERR_SUCCEED )
		return -7;

	return 0;
}

int main ( int argc, char ** argv )
{
	CoInitialize ( nullptr );
	esInitializeD3D11 ();

	int ret = DoProgram ();

	esUninitializeD3D11 ();
	CoUninitialize ();

	return ret;
}