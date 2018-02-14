Texture2D<float4> in_tex : register( t0 );
RWTexture2D<TARGET float4> out_tex : register( u0 );

SamplerState LinearSampler : register( s0 );

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void Rescale ( uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex )
{
	uint2 samplePos = groupID.xy * uint2 ( PROCESSINGUNIT, PROCESSINGUNIT ) + groupThreadID.xy;
	int2 textureSize;
	out_tex.GetDimensions ( textureSize.x, textureSize.y );

	[branch]
	if ( samplePos.x < textureSize.x && samplePos.y < textureSize.y )
	{
		float2 uv = ( samplePos + 0.5f ) / textureSize;
		out_tex [ samplePos ] = in_tex ( LinearSampler, uv );
	}
}