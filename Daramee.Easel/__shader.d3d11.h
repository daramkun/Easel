#define SHADER(code)										#code
const char * textureProcessingShaderSourceCode = SHADER (
Texture2D<INPUT_FORMAT> inputTexture : register ( t0 );
RWTexture2D<OUTPUT_FORMAT> outputTexture : register ( u0 );
cbuffer Filter : register ( b0 )
{
	int4 filter_radius;
	float4 filter [ 64 ];
};
static float filter_array [ 256 ] = ( float [ 256 ] ) filter;
SamplerState samplerState : register ( s0 );
RWStructuredBuffer<int> histogram : register ( u0 );

inline float4 RGB2YUV ( float4 rgb )
{
	return float4 (
		0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b,
		-0.147 * rgb.r - 0.289 * rgb.g + 0.436 * rgb.b,
		0.615 * rgb.r - 0.515 * rgb.g - 0.100 * rgb.b,
		rgb.a
	);
}

inline float4 YUV2RGB ( float4 yuv )
{
	return float4 (
		yuv.r + 1.140 * yuv.b,
		yuv.r - 0.395 * yuv.g - 0.581 * yuv.b,
		yuv.r + 2.032 * yuv.g,
		yuv.a
	);
}

inline float4 LoadPixel ( int3 xyz, int2 size )
{
	if ( xyz.x < 0 ) xyz.x = -xyz.x;
	if ( xyz.y < 0 ) xyz.y = -xyz.y;
	if ( xyz.x >= size.x ) xyz.x += ( size.x - xyz.x - 1 );
	if ( xyz.y >= size.y ) xyz.y += ( size.y - xyz.y - 1 );
	return inputTexture.Load ( xyz );
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void Filtering ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	int2 textureSize;
	inputTexture.GetDimensions ( textureSize.x, textureSize.y );

	float3 result = float3 ( 0, 0, 0 );
	int offset = 0;
	for ( int y = -filter_radius.y; y <= filter_radius.y; ++y )
	{
		for ( int x = -filter_radius.x; x <= filter_radius.x; ++x )
		{
			result += ( LoadPixel ( int3 ( dispatchThreadId ) + int3 ( x, y, 0 ),
				textureSize ).rgb * filter_array [ offset++ ] );
		}
	}

	outputTexture [ dispatchThreadId.xy ] =
		float4 ( result, inputTexture.Load ( dispatchThreadId ).a );
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void Copy ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ] = inputTexture.Load ( dispatchThreadId );
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void Resize ( uint3 groupId : SV_GroupID, uint3 dispatchThreadId : SV_DispatchThreadID,
	uint3 groupThreadId : SV_GroupThreadID )
{
	uint2 samplePos = groupId.xy * uint2 ( PROCESSINGUNIT, PROCESSINGUNIT ) + groupThreadId.xy;
	int2 textureSize;
	outputTexture.GetDimensions ( textureSize.x, textureSize.y );

	[branch]
	if ( samplePos.x < textureSize.x && samplePos.y < textureSize.y )
	{
		float2 uv = ( samplePos + 0.5f ) / textureSize;
		outputTexture [ samplePos ] = inputTexture.SampleLevel ( samplerState, uv, 0 );
	}
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void GetHistogram ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 color = inputTexture.Load ( dispatchThreadId );
	++histogram [ ( int ) ( RGB2YUV ( color ).r * 255 ) ];
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void HistogramEqualization ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 color = inputTexture.Load ( dispatchThreadId );
	float4 yuv = RGB2YUV ( color );
	yuv.y = histogram [ yuv.y ];
	outputTexture [ dispatchThreadId.xy ] = YUV2RGB ( yuv );
}
);