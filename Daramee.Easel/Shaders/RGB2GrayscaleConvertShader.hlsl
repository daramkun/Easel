Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

inline float4 RGB2Y ( float4 rgb )
{
	const float4 RGB2Y = float4 ( +0.299, +0.587, +0.114, 0.0 );
	const float pixel = dot ( rgb, RGB2Y );

	return float4 (
		pixel,
		0,
		0,
		rgb.a
		);
}

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ]
		= RGB2Y ( inputTexture.Load ( dispatchThreadId ) );
}