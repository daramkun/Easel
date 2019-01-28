Texture2D<float4> inputTexture : register ( t0 );
RWTexture2D<float4> outputTexture : register ( u0 );

cbuffer Filter : register ( b0 )
{
	int4 filter_radius;
	float4 filter [ 56 ];
};

static float filter_array [ 224 ] = ( float [ 224 ] ) filter;

inline float LoadPixel ( int3 xyz, int2 size )
{
	if ( xyz.x < 0 ) xyz.x = -xyz.x;
	if ( xyz.y < 0 ) xyz.y = -xyz.y;
	if ( xyz.x >= size.x ) xyz.x += ( size.x - xyz.x - 1 );
	if ( xyz.y >= size.y ) xyz.y += ( size.y - xyz.y - 1 );
	return inputTexture.Load ( xyz ).r;
}

[numthreads ( 16, 16, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	int2 textureSize;
	inputTexture.GetDimensions ( textureSize.x, textureSize.y );

	int offset = 0;
	float result = 0;
	for ( int y = -filter_radius.y; y <= filter_radius.y; ++y )
		for ( int x = -filter_radius.x; x <= filter_radius.x; ++x )
			result += ( LoadPixel ( int3 ( dispatchThreadId ) + int3 ( x, y, 0 ), textureSize ) * filter_array [ offset++ ] );

	outputTexture [ dispatchThreadId.xy ] =
		float4 ( result, inputTexture.Load ( dispatchThreadId ).gba );
}