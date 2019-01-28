Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

inline float4 RGB2YUV ( float4 rgb )
{
	const float3 RGB2Y = float3 ( +0.299, +0.587, +0.114 );
	const float3 RGB2U = float3 ( +0.596, -0.275, -0.321 );
	const float3 RGB2V = float3 ( +0.212, -0.523, +0.311 );

	return float4 (
		dot ( rgb.rgb, RGB2Y ),
		dot ( rgb.rgb, RGB2U ),
		dot ( rgb.rgb, RGB2V ),
		rgb.a
		);
}

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ]
		= RGB2YUV ( inputTexture.Load ( dispatchThreadId ) );
}