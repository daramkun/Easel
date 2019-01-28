Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

SamplerState samplerState : register ( s0 );

[numthreads ( 16, 16, 1 )]
void main ( uint3 groupId : SV_GroupID, uint3 dispatchThreadId : SV_DispatchThreadID,
	uint3 groupThreadId : SV_GroupThreadID )
{
	uint2 samplePos = groupId.xy * uint2 ( 16, 16 ) + groupThreadId.xy;
	int2 textureSize;
	outputTexture.GetDimensions ( textureSize.x, textureSize.y );

	[branch]
	if ( samplePos.x < ( uint ) textureSize.x && samplePos.y < ( uint ) textureSize.y )
	{
		float2 uv = ( samplePos + 0.5f ) / textureSize;
		outputTexture [ samplePos ] = inputTexture.SampleLevel ( samplerState, uv, 0 );
	}
}