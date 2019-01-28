Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

cbuffer Histogram : register ( b0 )
{
	int4 histogram [ 64 ];
};

static int histogram_array [ 256 ] = ( int [ 256 ] ) histogram;

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	float4 yuv = inputTexture.Load ( dispatchThreadId );
	yuv.r = histogram_array [ ( int ) floor ( yuv.r * 255 ) ] / 255.0f;
	outputTexture [ dispatchThreadId.xy ] = yuv;
}