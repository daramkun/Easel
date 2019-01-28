#include "Easel.h"

#include <memory>
#include <vector>
#include <wincodec.h>

#include <stdexcept>

easel::Filter::Filter ( float * filter, int x_diameter, int y_diameter )
	: x_radius ( x_diameter / 2 ), y_radius ( y_diameter / 2 )
	, width ( x_diameter ), height ( y_diameter )
{
	if ( filter == nullptr )
		throw std::runtime_error ( "Filter must not null." );
	if ( width >= 8 || height >= 8 )
		throw std::runtime_error ( "Filter radiuses must to be lesser than 8." );
	memset ( values, 0, sizeof ( values ) );
	memcpy ( values, filter, sizeof ( float ) * width * height );
}

easel::Filter easel::Filter::GetSharpenFilter ()
{
	float filter [] = {
		+0.00000000f, -0.66666666f, +0.00000000f,
		-0.66666666f, +3.66666666f, -0.66666666f,
		+0.00000000f, -0.66666666f, +0.00000000f, 
	};
	return Filter ( filter, 3, 3 );
}
