#ifndef __DARAMEE_EAGEL_H__
#define __DARAMEE_EAGEL_H__

#include <memory>
#include <exception>
#include <stdexcept>
#include <map>
#include <stdint.h>

#ifdef EAGEL_DLL
#	undef  EAGEL_DLL
#	define EAGEL_DLL										__declspec ( dllexport )
#else
#	define EAGEL_DLL										__declspec ( dllimport )
#endif

#if defined ( EAGEL_D3D11 )
#include <d3d11.h>
#endif

typedef int													EAGELERR;

extern "C"
{
	enum : EAGELERR
	{
		EAGELERR_SUCCEED = 0,
		EAGELERR_FAILED_INITIALIZE = -1,
		EAGELERR_ARGUMENT_IS_NULL = -2,
		EAGELERR_DEVICE_IS_NULL = -3,
		EAGELERR_UNAUTHORIZED_OBJECT = -4,
		EAGELERR_INVALID_OBJECT = -5,
		EAGELERR_SHADER_COMPILE_ERROR = -6,
		EAGELERR_UNSUPPORT_FORMAT = -7,
	};

	struct EAGEL_DLL EAGEL_FILTER
	{
	private:
		struct { int32_t x_radius, y_radius, width, height; } __declspec ( align ( 4 ) );
		float values [ 256 ];

	public:
		EAGEL_FILTER ( float * filter, int32_t x_radius, int32_t y_radius );
		EAGEL_FILTER ( int32_t x_radius, int32_t y_radius, ... );

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
		static EAGEL_FILTER getSharpenFilter ();

	} __declspec ( align ( 16 ) );

	enum EAGEL_DLL EAGEL_SAMPLING
	{
		EAGEL_SAMPLING_NEAREST,
		EAGEL_SAMPLING_LINEAR,
	};

#ifdef EAGEL_D3D11
	EAGELERR EAGEL_DLL egInitializeD3D11 ( ID3D11Device * d3dDevice = nullptr,
		unsigned processingUnit = 16 );
	void EAGEL_DLL egUninitializeD3D11 ();

	EAGELERR EAGEL_DLL egGetD3D11Device ( ID3D11Device ** result );

	EAGELERR EAGEL_DLL egDoFiltering (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		const EAGEL_FILTER & filter
	);
	EAGELERR EAGEL_DLL egDoCopyResource (
		ID3D11Texture2D * destination, ID3D11Texture2D * source
	);
	EAGELERR EAGEL_DLL egDoResize (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		EAGEL_SAMPLING sampling
	);
	EAGELERR EAGEL_DLL egDoHistogramEqualization (
		ID3D11Texture2D * destination, ID3D11Texture2D * source,
		const int histogram [ 256 ] = nullptr
	);
#endif

#ifdef EAGEL_D3D12

#endif

#ifdef EAGEL_OPENGL

#endif

#ifdef EAGEL_VULKAN

#endif

#ifdef EAGEL_METAL

#endif

#endif
}