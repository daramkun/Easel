#include <Easel.h>
#pragma comment ( lib, "Daramee.Easel.lib" )

int main ( void ) {

	CoInitialize ( nullptr );
	{
		CComPtr<easel::EaselFactory> factory;
		if ( FAILED ( easel::CreateEaselFactoryD3D11 ( &factory ) ) )
			return -1;

		CComPtr<easel::Bitmap> image, proceed;
		if ( FAILED ( factory->CreateBitmapFromFile ( L"Resources/11.jpg", &image ) ) )
			return -2;
		factory->Show ( image, "Loaded Image" );

		if ( FAILED ( factory->CreateBitmap ( image->Width (), image->Height (), &proceed ) ) )
			return -3;

		/*{
			CComPtr<easel::GammaSpaceBitmapProcessor> gamma;
			if ( FAILED ( factory->CreateGammaSpaceProcessor ( &gamma ) ) )
				return -4;
			gamma->SetGammaValue ( 2.2f, false );

			gamma->Process ( proceed, image );
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "From Gamma Space" );
		}*/

		/*{
			CComPtr<easel::BitmapProcessor> rgb8882rgb565;
			if ( FAILED ( factory->CreateRGB888SpaceToRGB565Space ( &rgb8882rgb565 ) ) )
				return -5;

			if ( !rgb8882rgb565->Process ( proceed, image ) )
				return -6;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "RGB888 to RGB565" );
		}*/

		{
			CComPtr<easel::BitmapProcessor> rgb2yuv;
			if ( FAILED ( factory->CreateRGB2YUVProcessor ( &rgb2yuv ) ) )
				return -5;

			if ( !rgb2yuv->Process ( proceed, image ) )
				return -6;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "RGB to YUV" );
		}

		int histogram [ 256 ];
		{
			/*CComPtr<easel::BitmapHistogramExtractor> histogramExtractor;
			if ( FAILED ( factory->CreateHistogramExtractor ( &histogramExtractor ) ) )
				return -7;

			histogramExtractor->SetBitmap ( image, true );

			if ( !histogramExtractor->Generate ( histogram ) )
				return -8;/**/

			/**/CComPtr<easel::ThresholdHistogramGenerator> histogramGenerator;
			if ( FAILED ( factory->CreateThresholdGenerator ( &histogramGenerator ) ) )
				return -7;

			easel::Threshold thresholds [] = {
				{ 5, 25 },
				{ 115, 40 },
				{ 255, 160 },
			};
			histogramGenerator->SetThreshold ( thresholds, _countof ( thresholds ) );

			if ( !histogramGenerator->Generate ( histogram ) )
				return -8;/**/
		}

		/*{
			CComPtr<easel::HistogramBitmapProcessor> histogramProcessor;
			if ( FAILED ( factory->CreateHistogramProcessor ( &histogramProcessor ) ) )
				return -9;

			histogramProcessor->SetHistogram ( histogram );
			if ( !histogramProcessor->Process ( proceed, image ) )
				return -10;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "Histogram Equalization" );
		}*/

		/*{
			CComPtr<easel::FilterBitmapProcessor> filterProcessor;
			if ( FAILED ( factory->CreateFilterProcessor ( &filterProcessor ) ) )
				return -11;

			easel::Filter filter = easel::Filter::GetSharpenFilter ();
			filterProcessor->SetFilter ( filter );

			if ( !filterProcessor->Process ( proceed, image ) )
				return -12;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "Sharpen Filtering" );
		}*/

		/*{
			CComPtr<easel::BitmapProcessor> saturateProcessor;
			if ( FAILED ( factory->CreateSaturateProcessor ( &saturateProcessor ) ) )
				return -13;

			if ( !saturateProcessor->Process ( proceed, image ) )
				return -14;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "Saturate" );
		}*/

		{
			CComPtr<easel::BitmapProcessor> yuv2rgb;
			if ( FAILED ( factory->CreateYUV2RGBProcessor ( &yuv2rgb ) ) )
				return -15;

			if ( !yuv2rgb->Process ( proceed, image ) )
				return -16;
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "YUV to RGB" );
		}

		/*{
			CComPtr<easel::GammaSpaceBitmapProcessor> gamma;
			if ( FAILED ( factory->CreateGammaSpaceProcessor ( &gamma ) ) )
				return -4;
			gamma->SetGammaValue ( 2.2f, true );

			gamma->Process ( proceed, image );
			factory->SwapBitmaps ( image, proceed );

			factory->Show ( image, "To Gamma Space" );
		}*/

		{
			CComPtr<IStream> outputStream;
			if ( FAILED ( SHCreateStreamOnFile ( L"1.jpg", STGM_WRITE | STGM_CREATE, &outputStream ) ) )
				return -17;
			if ( FAILED ( image->Encode ( outputStream ) ) )
				return -18;
		}
	}
	CoUninitialize ();

	return 0;
}