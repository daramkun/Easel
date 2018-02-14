Texture2D<float4> in_tex : register ( t0 );
RWTexture2D<TARGET float4> out_tex : register ( u0 );

cbuffer Filter : register ( b0 )
{
	int4 filter_radius;
	float4 filter [ 64 ];
};

static float filter_array [ 256 ] = ( float [ 256 ] ) filter;

inline float4 LoadPixel ( int3 xyz, int2 size )
{
	if ( xyz.x < 0 ) xyz.x = -xyz.x;
	if ( xyz.y < 0 ) xyz.y = -xyz.y;
	if ( xyz.x >= size.x ) xyz.x += ( size.x - xyz.x - 1 );
	if ( xyz.y >= size.y ) xyz.y += ( size.y - xyz.y - 1 );

	return in_tex.Load ( xyz );
}

inline float4 Filtering ( int3 pos, float2 size )
{
	float4 result = float4 ( 0, 0, 0, 0 );
	int offset = 0;
	for ( int y = -filter_radius.y; y <= filter_radius.y; ++y )
		for ( int x = -filter_radius.x; x <= filter_radius.x; ++x )
			result += LoadPixel ( pos + int3 ( x, y, 0 ), size ) * filter_array [ offset++ ];
	return result;
}

[numthreads ( PROCESSINGUNIT, PROCESSINGUNIT, 1 )]
void main ( uint3 dispatchThreadId : SV_DispatchThreadID )
{
	int2 textureSize;
	in_tex.GetDimensions ( textureSize.x, textureSize.y );

	float4 color = Filtering ( dispatchThreadId, textureSize );
	out_tex [ dispatchThreadId.xy ] = color;
}