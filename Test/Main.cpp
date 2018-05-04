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
	if ( esCreateCompatibleRotationTexture2D ( nullptr, sampleTexture, EASEL_ROTATION_270, &targetTexture ) != EASELERR_SUCCEED )
		return -3;
	/*if ( esCreateCompatibleScaleTexture2D ( nullptr, targetTexture, 75, &resizeTexture ) != EASELERR_SUCCEED )
		return -4;

	EASEL_FILTER filter = EASEL_FILTER::getSharpenFilter ( 100 );
	if ( esDoFiltering ( targetTexture, sampleTexture, filter ) != EASELERR_SUCCEED )
		return -5;

	if ( esDoResize ( resizeTexture, targetTexture, EASEL_SAMPLING_LINEAR ) )
		return -6;*/

	//if ( esDoHistogramEqualization ( targetTexture, sampleTexture, nullptr ) )
	//	return -7;
	/*float color [] = { 1, 1, 1, 0.5f };
	if ( esDoArithmeticOperation ( targetTexture, sampleTexture, color, EASEL_OPERATOR_MULTIPLY ) )
		return -7;*/
	if ( esDoRotation ( targetTexture, sampleTexture, EASEL_ROTATION_270 ) )
		return -7;

	if ( esSaveTexture2DToFile ( nullptr, L"Result.png", /*resizeTexture*/targetTexture ) != EASELERR_SUCCEED )
		return -8;

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