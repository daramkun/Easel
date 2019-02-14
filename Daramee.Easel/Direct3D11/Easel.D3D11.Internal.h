#ifndef __BITMAPCOMPRESSOR_D3D11_INTERNAL_H__
#define __BITMAPCOMPRESSOR_D3D11_INTERNAL_H__

#include <wincodec.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace easel
{
	namespace d3d11
	{
		class D3D11Bitmap;
		class D3D11ComputeShaderProcessor;
		class D3D11EaselFactory;

		class D3D11Bitmap : public easel::Bitmap
		{
		public:
			D3D11Bitmap ( D3D11EaselFactory * easelFactory, ID3D11Texture2D * texture, ID3D11ShaderResourceView * srv,
				ID3D11UnorderedAccessView * uav, ID3D11RenderTargetView * rtv );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual void GetSize ( UINT * width, UINT * height ) override;
			virtual HRESULT Encode ( IStream * stream, float quality = 0.75f ) override;

		private:
			volatile ULONG _refCount;

		public:
			CComPtr<D3D11EaselFactory> _factory;

			CComPtr<ID3D11Texture2D> _texture;
			CComPtr<ID3D11ShaderResourceView> _srv;
			CComPtr<ID3D11UnorderedAccessView> _uav;
			CComPtr<ID3D11RenderTargetView> _rtv;
		};

		class D3D11ComputeShaderProcessor : public easel::ComputeShaderBitmapProcessor
		{
		public:
			D3D11ComputeShaderProcessor ( D3D11EaselFactory * factory, ID3D11ComputeShader * computeShader, 
				ID3D11Buffer * constantBuffer, UINT constantBufferSize );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( easel::Bitmap * destination, easel::Bitmap * source ) override;
			virtual BOOL CheckBitmapSize ( easel::Bitmap * b1, easel::Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;

		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;

		private:
			volatile ULONG _refCount;

		public:
			CComPtr<D3D11EaselFactory> _factory;

			CComPtr<ID3D11ComputeShader> _shader;
			CComPtr<ID3D11Buffer> _constantBuffer;
			UINT _constantBufferSize;
		};

		class D3D11FilterProcessor : public easel::FilterBitmapProcessor
		{
		public:
			D3D11FilterProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;
			
		public:
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;
			
		public:
			virtual void SetFilter ( const Filter & filter ) override;

		private:
			ULONG _refCount;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
		};

		struct GammaSpaceArgs { float gamma, reserved1, reserved2, reserved3; };

		class D3D11GammaProcessor : public easel::GammaSpaceBitmapProcessor
		{
		public:
			D3D11GammaProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;

		public:
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;

		public:
			virtual void SetGammaValue ( float gamma = 2.2f, bool toGammaSpace = false ) override;

		private:
			ULONG _refCount;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
		};

		struct RotationArgs { int rot, reserved1, reserved2, reserved3; };

		class D3D11RotationProcessor : public easel::RotationBitmapProcessor
		{
		public:
			D3D11RotationProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;
			
		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;
			
		public:
			virtual void SetRotationAngle ( Rotation rot ) override;

		private:
			ULONG _refCount;
			Rotation rot;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
		};

		class D3D11ResizeProcessor : public easel::ResizeBitmapProcessor
		{
		public:
			D3D11ResizeProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;
			
		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;
			
		public:
			virtual void SetSamplingFilter ( SamplingFilter filter ) override;

		private:
			ULONG _refCount;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
			CComPtr<ID3D11SamplerState> _samplerState;
		};

		struct ArithmeticOp { float r, g, b, a; int op; int reserved1, reserved2, reserved3; };

		class D3D11ArithmeticProcessor : public easel::ArithmeticBitmapProcessor
		{
		public:
			D3D11ArithmeticProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;

		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;

		public:
			virtual void SetOperation ( Operator op, float rgba [ 4 ] ) override;

		private:
			ULONG _refCount;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
		};

		class D3D11HistogramProcessor : public easel::HistogramBitmapProcessor
		{
		public:
			D3D11HistogramProcessor ( D3D11ComputeShaderProcessor * processor );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Process ( Bitmap * destination, Bitmap * source ) override;

		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;
			virtual BOOL CheckBitmapSize ( Bitmap * b1, Bitmap * b2 ) override;
			virtual void SetConstantBufferData ( const void * data ) override;
			virtual void SetAdditionalState () override;

		public:
			virtual void SetHistogram ( int histogram [ 256 ] ) override;

		private:
			ULONG _refCount;

		public:
			CComPtr<D3D11ComputeShaderProcessor> _processor;
		};

		class D3D11HistogramExtractor : public BitmapHistogramExtractor
		{
		public:
			D3D11HistogramExtractor ( D3D11EaselFactory * factory,
				ID3D11ComputeShader * shader,
				ID3D11Buffer * buffer, ID3D11UnorderedAccessView * bufferUAV,
				ID3D11Buffer * copyOnly );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Generate ( int buffer [ 256 ] ) override;

		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;

		public:
			virtual void SetBitmap ( Bitmap * bitmap, bool autoCorrection ) override;

		private:
			ULONG _refCount;
			CComPtr<D3D11EaselFactory> _factory;

			CComPtr<D3D11Bitmap> _bitmap;
			bool _autoCorrection;

		public:
			CComPtr<ID3D11ComputeShader> _shader;
			CComPtr<ID3D11Buffer> _buffer;
			CComPtr<ID3D11UnorderedAccessView> _bufferUAV;
			CComPtr<ID3D11Buffer> _copyOnly;
		};

		class D3D11ThresholdHistogramGenerator : public easel::ThresholdHistogramGenerator
		{
		public:
			D3D11ThresholdHistogramGenerator ( D3D11EaselFactory * factory );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual BOOL Generate ( int buffer [ 256 ] ) override;

		public:
			virtual HRESULT GetFactory ( EaselFactory ** factory ) override;

		public:
			virtual void SetThreshold ( Threshold * thresholds, int thresholdCount ) override;

		private:
			ULONG _refCount;
			CComPtr<D3D11EaselFactory> _factory;

		public:
			Threshold _thresholds [ 256 ];
			int _thresholdCount;
		};

		class D3D11EaselFactory : public easel::EaselFactory
		{
		public:
			D3D11EaselFactory ( IWICImagingFactory * wicImagingFactory, ID3D11Device * d3dDevice, ID3D11DeviceContext * immediateContext );

		public:
			virtual HRESULT __stdcall QueryInterface ( REFIID riid, void ** ppvObject ) override;
			virtual ULONG __stdcall AddRef ( void ) override;
			virtual ULONG __stdcall Release ( void ) override;

		public:
			virtual HRESULT CreateBitmap ( UINT width, UINT height, easel::Bitmap ** bitmap ) override;
			virtual HRESULT CreateBitmapFromFile ( LPCWSTR filename, easel::Bitmap ** bitmap ) override;
			virtual HRESULT CopyBitmap ( easel::Bitmap * destination, easel::Bitmap * source ) override;
			virtual void SwapBitmaps ( easel::Bitmap * _b1, easel::Bitmap * _b2 ) override;

		public:
			virtual HRESULT CreateComputeShaderProcessor ( LPCSTR shaderCode, bool useConstantBuffer, int constantBufferSize, easel::ComputeShaderBitmapProcessor ** processor ) override;
			virtual HRESULT CreateComputeShaderProcessor ( LPCWSTR filename, bool useConstantBuffer, int constantBufferSize, easel::ComputeShaderBitmapProcessor ** processor ) override;

			virtual HRESULT CreateSaturateProcessor ( easel::BitmapProcessor ** processor ) override;
			virtual HRESULT CreateRGB2YUVProcessor ( easel::BitmapProcessor ** processor ) override;
			virtual HRESULT CreateYUV2RGBProcessor ( easel::BitmapProcessor ** processor ) override;
			virtual HRESULT CreateRGB2GrayscaleProcessor ( easel::BitmapProcessor ** processor ) override;
			virtual HRESULT CreateRGB888SpaceToRGB565Space ( BitmapProcessor ** processor ) override;

			virtual HRESULT CreateFilterProcessor ( easel::FilterBitmapProcessor ** processor ) override;
			virtual HRESULT CreateGammaSpaceProcessor ( easel::GammaSpaceBitmapProcessor ** processor ) override;
			virtual HRESULT CreateRotationProcessor ( easel::RotationBitmapProcessor ** processor ) override;
			virtual HRESULT CreateResizeProcessor ( easel::ResizeBitmapProcessor ** processor ) override;
			virtual HRESULT CreateArithmeticProcessor ( easel::ArithmeticBitmapProcessor ** processor ) override;
			virtual HRESULT CreateHistogramProcessor ( easel::HistogramBitmapProcessor ** processor ) override;

		public:
			virtual HRESULT CreateHistogramExtractor ( easel::BitmapHistogramExtractor ** generator ) override;
			virtual HRESULT CreateThresholdGenerator ( easel::ThresholdHistogramGenerator ** generator ) override;

		public:
			virtual void Show ( easel::Bitmap * bitmap, LPCSTR title ) override;

		private:
			volatile ULONG _refCount;

		public:
			CComPtr<IWICImagingFactory> _wicImagingFactory;
			CComPtr<ID3D11Device> _d3dDevice;
			CComPtr<ID3D11DeviceContext> _immediateContext;
		};
	}
}

#endif