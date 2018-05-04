#define SHADER(code)										#code
const char * textureProcessingShaderSourceCode = SHADER (
Texture2D<INPUT_FORMAT> inputTexture : register ( t0 );
RWTexture2D<OUTPUT_FORMAT> outputTexture : register ( u0 );
cbuffer Filter : register ( b0 )
{
	int4 filter_radius;
	float4 filter [ 64 ];
};
cbuffer ArithmeticOperation : register ( b1 )
{
	float4 compute_color;
	int compute_operator;
};
cbuffer RotationOperation : register ( b2 )
{
	int rotation;
};
static float filter_array [ 256 ] = ( float [ 256 ] ) filter;
SamplerState samplerState : register ( s0 );
RWStructuredBuffer<int> histogram : register ( u1 );

inline float4 RGB2YUV ( float4 rgb )
{
	const float4 RGB2Y = float4 ( +0.299, +0.587, +0.114, 0.0 );
	const float4 RGB2U = float4 ( +0.596, -0.275, -0.321, 0.0 );
	const float4 RGB2V = float4 ( +0.212, -0.523, +0.311, 0.0 );

	return float4 (
		dot ( rgb, RGB2Y ),
		dot ( rgb, RGB2U ),
		dot ( rgb, RGB2V ),
		rgb.a
	);
}
inline float RGB2Y ( float4 rgb )
{
	const float4 RGB2Y = float4 ( +0.299, +0.587, +0.114, 0.0 );
	return dot ( rgb, RGB2Y );
}
inline float4 YUV2RGB ( float4 yuv )
{
	const float4 YUV2R = float4 ( +1.0, +0.956, +0.621, 0.0 );
	const float4 YUV2G = float4 ( +1.0, -0.272, -0.647, 0.0 );
	const float4 YUV2B = float4 ( +1.0, -1.107, +1.704, 0.0 );

	return float4 (
		dot ( yuv, YUV2R ),
		dot ( yuv, YUV2G ),
		dot ( yuv, YUV2B ),
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
	int y = ( int ) ( RGB2Y ( inputTexture.Load ( dispatchThreadId ) ) * 255 );
	InterlockedAdd ( histogram [ y ], 1 );
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void HistogramEqualization ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 yuv = RGB2YUV ( inputTexture.Load ( dispatchThreadId ) );
	yuv.r = histogram [ ( int ) ( yuv.r * 255 ) ] / 255.0f;
	outputTexture [ dispatchThreadId.xy ] = YUV2RGB ( yuv );
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void ArithmeticOperation ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 color = inputTexture.Load ( dispatchThreadId );
	if ( compute_operator == 0 )
		outputTexture [ dispatchThreadId.xy ] = color + compute_color;
	else if ( compute_operator == 1 )
		outputTexture [ dispatchThreadId.xy ] = color - compute_color;
	else if ( compute_operator == 2 )
		outputTexture [ dispatchThreadId.xy ] = color * compute_color;
	else if ( compute_operator == 3 )
		outputTexture [ dispatchThreadId.xy ] = color / compute_color;
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void RotationOperation ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	int2 textureSize;
	inputTexture.GetDimensions ( textureSize.x, textureSize.y );

	switch ( rotation )
	{
		case 1:
			{
				outputTexture [ dispatchThreadId.xy ] = inputTexture.Load ( int3 ( textureSize.x - dispatchThreadId.y - 1, dispatchThreadId.x, dispatchThreadId.z ) );
			}
			break;
		case 2:
			{
				outputTexture [ dispatchThreadId.xy ] = inputTexture.Load ( int3 ( textureSize - dispatchThreadId.xy - int3 ( 1, 1, 0 ), dispatchThreadId.z ) );
			}
			break;
		case 3:
			{
				outputTexture [ dispatchThreadId.xy ] = inputTexture.Load ( int3 ( dispatchThreadId.y, textureSize.y - dispatchThreadId.x - 1, dispatchThreadId.z ) );
			}
			break;
	}
}

);