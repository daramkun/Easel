Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

static int3 ToRGB565 = int3 ( 31, 63, 31 );
static float3 ToRGB888 = float3 ( 0.03225806451612903225806451612903f, 0.01587301587301587301587301587302f, 0.03225806451612903225806451612903f );

float4 RGB8882RGB565 ( float4 rgba )
{
	return float4 ( ( ( int3 ) ( rgba.rgb * ToRGB565 ) ) * ToRGB888, rgba.a );
}

[numthreads ( 16, 16, 1 )]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	outputTexture [ dispatchThreadId.xy ]
		= RGB8882RGB565 ( inputTexture.Load ( dispatchThreadId ) );
}