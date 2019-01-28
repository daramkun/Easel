Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

cbuffer ArithmeticOperation : register ( b0 )
{
	float4 compute_color;
	int compute_operator;
};

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 color = inputTexture.Load ( dispatchThreadId );

	[branch]
	switch ( compute_operator )
	{
		case 0: outputTexture [ dispatchThreadId.xy ] = color + compute_color; break;
		case 1: outputTexture [ dispatchThreadId.xy ] = color - compute_color; break;
		case 2: outputTexture [ dispatchThreadId.xy ] = color * compute_color; break;
		case 3: outputTexture [ dispatchThreadId.xy ] = color / compute_color; break;
	}
}