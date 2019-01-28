Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

cbuffer Gamma : register ( b0 )
{
	float gamma;
}

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ]
		= pow ( inputTexture.Load ( dispatchThreadId ), gamma );
}