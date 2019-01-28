Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

cbuffer RotationOperation : register ( b0 )
{
	int rotation;
};

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	int2 textureSize;
	inputTexture.GetDimensions ( textureSize.x, textureSize.y );

	switch ( rotation )
	{
		case 0:
			outputTexture [ dispatchThreadId.xy ]
				= inputTexture.Load ( dispatchThreadId );
			break;

		case 1:
			outputTexture [ dispatchThreadId.xy ]
				= inputTexture.Load ( int3 ( textureSize.x - dispatchThreadId.y - 1, dispatchThreadId.x, dispatchThreadId.z ) );
			break;

		case 2:
			outputTexture [ dispatchThreadId.xy ]
				= inputTexture.Load ( int3 ( textureSize - dispatchThreadId.xy - int2 ( 1, 1 ), dispatchThreadId.z ) );
			break;

		case 3:
			outputTexture [ dispatchThreadId.xy ]
				= inputTexture.Load ( int3 ( dispatchThreadId.y, textureSize.y - dispatchThreadId.x - 1, dispatchThreadId.z ) );
			break;
	}
}