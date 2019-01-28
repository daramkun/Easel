Texture2D<float4> inputTexture : register ( t0 );
RWStructuredBuffer<int> histogram : register ( u0 );

static uint2 ThreadUnit = uint2 ( 16, 16 );

[numthreads ( 16, 16, 1 )]
void main ( uint3 groupId : SV_GroupID, uint3 dispatchThreadId : SV_DispatchThreadID,
	uint3 groupThreadId : SV_GroupThreadID )
{
	uint2 samplePos = groupId.xy * ThreadUnit + groupThreadId.xy;
	int2 textureSize;
	inputTexture.GetDimensions ( textureSize.x, textureSize.y );

	[branch]
	if ( samplePos.x > ( uint ) textureSize.x || samplePos.y > ( uint ) textureSize.y )
		return;

	InterlockedAdd ( histogram [ ( int ) ( inputTexture.Load ( dispatchThreadId ).r * 255 ) ], 1 );
}