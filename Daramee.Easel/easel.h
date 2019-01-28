#ifndef __DARAMEE_BITMAPCOMPRESSOR_H__
#define __DARAMEE_BITMAPCOMPRESSOR_H__

#include <Windows.h>
#include <atlbase.h>

namespace easel
{
	class Bitmap;

	class BitmapProcessor;
	class ComputeShaderBitmapProcessor;
	class FilterBitmapProcessor;
	class GammaSpaceBitmapProcessor;
	class RotationBitmapProcessor;
	class ResizeBitmapProcessor;
	class ArithmeticBitmapProcessor;
	class HistogramBitmapProcessor;

	class HistogramGenerator;
	class BitmapHistogramExtractor;
	class ThresholdHistogramGenerator;

	////////////////////////////////////////////////////////
	// Easel Object Factory class
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "1AD0E4D8-99A4-4509-A197-EDB7FA6092DF" ) )
		EaselFactory : public IUnknown
	{
	public:
		virtual HRESULT CreateBitmap ( UINT width, UINT height, easel::Bitmap ** bitmap ) PURE;
		virtual HRESULT CreateBitmapFromFile ( LPCWSTR filename, Bitmap ** bitmap ) PURE;
		virtual HRESULT CopyBitmap ( Bitmap * destination, Bitmap * source ) PURE;
		virtual void Swap ( Bitmap * b1, Bitmap * b2 ) PURE;

	public:
		virtual HRESULT CreateComputeShaderProcessor ( LPCSTR shaderCode, bool useConstantBuffer, int constantBufferSize, ComputeShaderBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateComputeShaderProcessor ( LPCWSTR filename, bool useConstantBuffer, int constantBufferSize, ComputeShaderBitmapProcessor ** processor ) PURE;

		virtual HRESULT CreateSaturateProcessor ( BitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateRGB2YUVProcessor ( BitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateYUV2RGBProcessor ( BitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateRGB2GrayscaleProcessor ( BitmapProcessor ** processor ) PURE;

		virtual HRESULT CreateFilterProcessor ( FilterBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateGammaSpaceProcessor ( GammaSpaceBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateRotationProcessor ( RotationBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateResizeProcessor ( ResizeBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateArithmeticProcessor ( ArithmeticBitmapProcessor ** processor ) PURE;
		virtual HRESULT CreateHistogramProcessor ( HistogramBitmapProcessor ** processor ) PURE;

	public:
		virtual HRESULT CreateHistogramExtractor ( BitmapHistogramExtractor ** generator ) PURE;
		virtual HRESULT CreateThresholdGenerator ( ThresholdHistogramGenerator ** generator ) PURE;

	public:
		virtual void Show ( Bitmap * bitmap, LPCSTR title = "Easel Debug Window" ) PURE;
	};

	////////////////////////////////////////////////////////
	// Create Direct3D 11 Hardware Accelerated Factory
	////////////////////////////////////////////////////////
	HRESULT CreateEaselFactoryD3D11 ( EaselFactory ** factory );

	////////////////////////////////////////////////////////
	// Bitmap class
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "C6CE5219-8CC4-43AB-8FCA-717F139A89C2" ) )
		Bitmap : public IUnknown
	{
	public:
		virtual void GetSize ( UINT * width, UINT * height ) PURE;
		inline UINT Width () { UINT width; GetSize ( &width, nullptr ); return width; }
		inline UINT Height () { UINT height; GetSize ( nullptr, &height ); return height; }

	public:
		virtual HRESULT Encode ( IStream * stream, float quality = 0.75f ) PURE;
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor Interface
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "EBE21917-D948-420F-A803-58DE77881D96" ) )
		BitmapProcessor : public IUnknown
	{
	public:
		virtual ~BitmapProcessor () { }

	public:
		virtual BOOL Process ( Bitmap * destination, Bitmap * source ) PURE;

	public:
		virtual HRESULT GetFactory ( EaselFactory ** factory ) PURE;
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor Interface with Compute Shader
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "8BF6430B-B987-4422-B073-ADF51F965AF9" ) )
		ComputeShaderBitmapProcessor : public BitmapProcessor
	{
	protected:
		virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) PURE;

	public:
		virtual void SetConstantBufferData ( const void * data ) PURE;
		virtual void SetAdditionalState () PURE;
	};

	////////////////////////////////////////////////////////
	// N*N Filter
	////////////////////////////////////////////////////////
	struct Filter
	{
	private:
		struct { int x_radius, y_radius, width, height; } __declspec ( align ( 4 ) );
		__declspec ( align ( 4 ) ) float values [ 256 ];

	public:
		Filter ( float * filter, int x_diameter, int y_diameter );

		const float * GetValues () const { return values; }
		int GetRadiusX () const { return x_radius; }
		int GetRadiusY () const { return y_radius; }
		int GetWidth () const { return width; }
		int GetHeight () const { return height; }

	public:
		void Add ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] += scalar; }
		void Subtract ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] -= scalar; }
		void Multiply ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] *= scalar; }
		void Divide ( float scalar ) { int len = width * height; float temp = 1 / scalar; for ( int i = 0; i < len; ++i ) values [ i ] *= temp; }

	public:
		static Filter GetSharpenFilter ();
	} __declspec ( align ( 16 ) );

	////////////////////////////////////////////////////////
	// Bitmap Processor via N*N Filter
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "7E2C265F-5D7C-4517-91A1-171045F96E60" ) )
		FilterBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetFilter ( const Filter & filter ) PURE;
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor for Gamma
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "381D6D66-1EEA-4C67-AB9B-8FF567A72564" ) )
		GammaSpaceBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetGammaValue ( float gamma = 2.2f, bool toGammaSpace = false ) PURE;
	};

	////////////////////////////////////////////////////////
	// Rotation Angle
	////////////////////////////////////////////////////////
	enum Rotation
	{
		kRotation_0,
		kRotation_90,
		kRotation_180,
		kRotation_270,
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor for Rotation
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "8A285DF9-E178-462B-B295-ACC0A2FB10F7" ) )
		RotationBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetRotationAngle ( Rotation rot ) PURE;
	};

	////////////////////////////////////////////////////////
	// Bitmap Sampling method
	////////////////////////////////////////////////////////
	enum SamplingFilter
	{
		kSamplingFilter_Nearest,
		kSamplingFilter_Linear,
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor for Resize
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "FF2F0CC5-27FA-4F51-955B-9F0929520CCC" ) )
		ResizeBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetSamplingFilter ( SamplingFilter filter ) PURE;
	};

	////////////////////////////////////////////////////////
	// Arithmetic Operator
	////////////////////////////////////////////////////////
	enum Operator
	{
		kOperator_Add,
		kOperator_Subtract,
		kOperator_Multiply,
		kOperator_Divide,
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor for Arithmetic Operation
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "2FB59BE0-BB60-4AFD-AF15-5DF6FB054F0C" ) )
		ArithmeticBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetOperation ( Operator op, float rgba [ 4 ] ) PURE;
	};

	////////////////////////////////////////////////////////
	// Bitmap Processor for Histogram Equalization
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "6670E8F7-C767-41D1-99A0-9C00A93629A9" ) )
		HistogramBitmapProcessor : public ComputeShaderBitmapProcessor
	{
	public:
		virtual void SetHistogram ( int histogram [ 256 ] ) PURE;
	};

	////////////////////////////////////////////////////////
	// Histogram Generator Interface
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "6CA13801-C267-4892-9092-6E82F332600E" ) )
		HistogramGenerator : public IUnknown
	{
	public:
		virtual BOOL Generate ( int buffer [ 256 ] ) PURE;

	public:
		virtual HRESULT GetFactory ( EaselFactory ** factory ) PURE;
	};

	////////////////////////////////////////////////////////
	// Histogram Generator for Equalization
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "60BF8B54-8669-4047-A866-9A1C1999A463" ) )
		BitmapHistogramExtractor : public HistogramGenerator
	{
	public:
		virtual void SetBitmap ( Bitmap * bitmap, bool autoCorrection ) PURE;
	};

	////////////////////////////////////////////////////////
	// Threshold Information
	////////////////////////////////////////////////////////
	struct Threshold
	{
	public:
		int thresholdY;
		int thresholdWeight;
	};

	////////////////////////////////////////////////////////
	// Histogram Generator for Threshold
	////////////////////////////////////////////////////////
	class __declspec ( uuid ( "DFFB9187-F3A1-4AE7-BE58-0DF3CC25C30A" ) )
		ThresholdHistogramGenerator : public HistogramGenerator
	{
	public:
		virtual void SetThreshold ( Threshold * thresholds, int thresholdCount ) PURE;
	};
}

#endif