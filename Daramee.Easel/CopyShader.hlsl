Texture2D<float4> in_tex : register ( t0 );
RWTexture2D<TARGET float4> out_tex : register ( u0 );
[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	out_tex [ dispatchThreadId.xy ] = in_tex.Load ( dispatchThreadId );
}