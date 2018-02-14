#include "easel.h"

EAGEL_FILTER::EAGEL_FILTER ( float * filter, int32_t x_radius, int32_t y_radius )
	: x_radius ( x_radius ), y_radius ( y_radius )
	, width ( x_radius * 2 + 1 ), height ( y_radius * 2 + 1 )
{
	if ( filter == nullptr )
		throw std::runtime_error ( "Filter must not null." );
	if ( width >= 8 || height >= 8 )
		throw std::runtime_error ( "Filter radiuses must to be lesser than 8." );
	memcpy ( values, filter, sizeof ( float ) * width * height );
}

EAGEL_FILTER::EAGEL_FILTER ( int32_t x_radius, int32_t y_radius, ... )
	: x_radius ( x_radius ), y_radius ( y_radius )
	, width ( x_radius * 2 + 1 ), height ( y_radius * 2 + 1 )
{
	va_list valist;

	if ( width >= 8 || height >= 8 )
		throw std::runtime_error ( "Filter radiuses must to be lesser than 8." );

	va_start ( valist, y_radius );

	int length = width * height;
	for ( int i = 0; i < length; ++i )
		values [ i ] = ( float ) va_arg ( valist, double );

	va_end ( valist );
}

void EAGEL_FILTER::add ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] += scalar; }
void EAGEL_FILTER::subtract ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] -= scalar; }
void EAGEL_FILTER::multiply ( float scalar ) { int len = width * height; for ( int i = 0; i < len; ++i ) values [ i ] *= scalar; }
void EAGEL_FILTER::divide ( float scalar ) { int len = width * height; float temp = 1 / scalar; for ( int i = 0; i < len; ++i ) values [ i ] *= temp; }

EAGEL_FILTER EAGEL_FILTER::getSharpenFilter ()
{
	EAGEL_FILTER sharpenFilter ( 1, 1,
		0, -0.66666666f, 0,
		-0.66666666f, 3.66666666f, -0.66666666f,
		0, -0.66666666f, 0
	);
	return sharpenFilter;
}