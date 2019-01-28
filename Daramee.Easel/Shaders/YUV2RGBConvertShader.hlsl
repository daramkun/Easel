Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

inline float4 YUV2RGB ( float4 yuv )
{
	const float3 YUV2R = float3 ( +1.0, +0.956, +0.621 );
	const float3 YUV2G = float3 ( +1.0, -0.272, -0.647 );
	const float3 YUV2B = float3 ( +1.0, -1.107, +1.704 );

	return float4 (
		dot ( yuv.rgb, YUV2R ),
		dot ( yuv.rgb, YUV2G ),
		dot ( yuv.rgb, YUV2B ),
		yuv.a
		);
}

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ]
		= YUV2RGB ( inputTexture.Load ( dispatchThreadId ) );
}