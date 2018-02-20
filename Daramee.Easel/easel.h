#ifndef __DARAMEE_EAGEL_H__
#define __DARAMEE_EASEL_H__

#include <memory>
#include <exception>
#include <stdexcept>
#include <map>
#include <stdint.h>

#ifdef EASEL_DLL
#	undef  EASEL_DLL
#	define EASEL_DLL										__declspec ( dllexport )
#else
#	define EASEL_DLL										__declspec ( dllimport )
#endif

#if defined ( EASEL_D3D11 )
#include <d3d11.h>
#endif

typedef int													EASELERR;

extern "C"
{
	enum : EASELERR
	{
		EASELERR_SUCCEED = 0,
		EASELERR_FAILED_INITIALIZE = -1,
		EASELERR_ARGUMENT_IS_NULL = -2,
		EASELERR_DEVICE_IS_NULL = -3,
		EASELERR_UNAUTHORIZED_OBJECT = -4,
		EASELERR_INVALID_OBJECT = -5,
		EASELERR_SHADER_COMPILE_ERROR = -6,
		EASELERR_UNSUPPORT_FORMAT = -7,
		EASELERR_FILE_IS_NOT_FOUND = -8,
	};

	struct EASEL_DLL EASEL_FILTER
	{
	private:
		struct { int32_t x_radius, y_radius, width, height; } __declspec ( align ( 4 ) );
		float values [ 256 ];

	public:
		EASEL_FILTER ( float * filter, int32_t x_radius, int32_t y_radius );
		EASEL_FILTER ( int32_t x_radius, int32_t y_radius, ... );

		const float * getValues () const { return values; }
		int32_t getRadiusX () const { return x_radius; }
		int32_t getRadiusY () const { return y_radius; }
		int32_t getWidth () const { return width; }
		int32_t getHeight () const { return height; }

	public:
		void add ( float scalar );
		void subtract ( float scalar );
		void multiply ( float scalar );
		void divide ( float scalar );

	public:
		static EASEL_FILTER getSharpenFilter ();

	} __declspec ( align ( 16 ) );

	enum EASEL_DLL EASEL_SAMPLING
	{
		EASEL_SAMPLING_NEAREST,
		EASEL_SAMPLING_LINEAR,
	};

	enum EASEL_DLL EASEL_OPERATOR
	{
		EASEL_OPERATOR_ADD,
		EASEL_OPERATOR_SUBTRACT,
		EASEL_OPERATOR_MULTIPLY,
		EASEL_OPERATOR_DIVIDE,
	};

#ifdef EASEL_D3D11
	EASELERR EASEL_DLL esInitializeD3D11 ( ID3D11Device * d3dDevice = nullptr,
		unsigned processingUnit = 16 );
	void EASEL_DLL esUninitializeD3D11 ();

	EASELERR EASEL_DLL esGetD3D11Device ( ID3D11Device ** result );

	EASELERR EASEL_DLL esLoadTexture2DFromStream ( ID3D11Device * d3dDevice,
		IStream * stream, DXGI_FORMAT format, ID3D11Texture2D ** result );
	EASELERR EASEL_DLL esLoadTexture2DFromFile ( ID3D11Device * d3dDevice,
		LPCTSTR filename, DXGI_FORMAT format, ID3D11Texture2D ** result );
	EASELERR EASEL_DLL esCreateCompatibleTexture2D ( ID3D11Device * d3dDevice,
		ID3D11Texture2D * original, ID3D11Texture2D ** result );
	EASELERR EASEL_DLL esCreateCompatibleScaleTexture2D ( ID3D11Device * d3dDevice,
		ID3D11Texture2D * original, int scale, ID3D11Texture2D ** result );
	EASELERR EASEL_DLL esSaveTexture2DToStream ( ID3D11Device * d3dDevice,
		IStream * stream, ID3D11Texture2D * target );
	EASELERR EASEL_DLL esSaveTexture2DToFile ( ID3D11Device * d3dDevice,
		LPCTSTR filename, ID3D11Texture2D * target );

	EASELERR EASEL_DLL esDoFiltering (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		const EASEL_FILTER & filter
	);
	EASELERR EASEL_DLL esDoCopyResource (
		ID3D11Texture2D * destination, ID3D11Texture2D * source
	);
	EASELERR EASEL_DLL esDoResize (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		EASEL_SAMPLING sampling
	);
	EASELERR EASEL_DLL esDoHistogramEqualization (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		const int histogram [ 256 ] = nullptr
	);
	EASELERR EASEL_DLL esDoArithmeticOperation (
		ID3D11Texture2D * destination, ID3D11Texture2D * source, 
		float color [ 4 ], EASEL_OPERATOR op
	);
#endif

#ifdef EASEL_D3D12

#endif

#ifdef EASEL_OPENGL

#endif

#ifdef EASEL_VULKAN

#endif

#ifdef EASEL_METAL

#endif

#endif
}