/* ColorConv.h
 *
 * Copyright (c) 2010, IMURA Masataka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	__COLOR_CONV_H__
#define	__COLOR_CONV_H__

#define	COLOR_CONV_RANGE_CHECK	1

#if COLOR_CONV_RANGE_CHECK
#include	<iostream>
#endif

template<class T>
class	ColorConv {
private:
	T	rgb_range;
	T	h_range, s_range, v_range;

public:
    ColorConv( T rgb = 1, T h = 1, T s = 1, T v = 1 )
		: rgb_range( rgb ), h_range( h ), s_range( s ), v_range( v )
	{
	}
	
    ~ColorConv()
	{
	}
	
    void	RgbToHsv( const T r, const T g, const T b,
					  T& h, T& s, T& v );
    void	HsvToRgb( const T h0, const T s0, const T v0,
					  T& r, T& g, T& b );
};

template<class T>
void
ColorConv<T>::RgbToHsv( const T r, const T g, const T b, T& h, T& s, T& v )
{
	T	rgb[ 3 ];
	int	region;
	
	if ( b <= g ) {
		if ( g <= r ) {			// r > g > b: red - yellow
			rgb[ 0 ] = b;
			rgb[ 1 ] = g;
			rgb[ 2 ] = r;
			region = 0;
		} else if ( b <= r ) {	// g > r > b: yellow - green
			rgb[ 0 ] = b;
			rgb[ 1 ] = r;
			rgb[ 2 ] = g;
			region = 1;
		} else {				// g > b > r: green - blue
			rgb[ 0 ] = r;
			rgb[ 1 ] = b;
			rgb[ 2 ] = g;
			region = 2;
		}
	} else {
		if ( r <= g ) {			// b > g > r: blue - cyan
			rgb[ 0 ] = r;
			rgb[ 1 ] = g;
			rgb[ 2 ] = b;
			region = 3;
		} else if ( r <= b ) {	// b > r > g: cyan - magenta
			rgb[ 0 ] = g;
			rgb[ 1 ] = r;
			rgb[ 2 ] = b;
			region = 4;
		} else {				// r > b > g: magenta - red
			rgb[ 0 ] = g;
			rgb[ 1 ] = b;
			rgb[ 2 ] = r;
			region = 5;
		}
	}

	// rgb[ 0 ] = min, rgb[ 2 ] = max

	if ( rgb[ 2 ] == 0 ) {
		h = 0;
		s = 0;
		v = 0;
	} else if ( rgb[ 0 ] == rgb[ 2 ] ) {
		h = 0;
		s = 0;
		v = ( rgb[ 2 ] * v_range ) / rgb_range;
	} else {
		int	lsb = region & 1;
		int	sign = 1 - 2 * lsb;
		h = ( region + lsb ) * ( h_range / 6 )
			+ sign * (( rgb[ 1 ] - rgb[ 0 ] ) * h_range )
			/ (( rgb[ 2 ] - rgb[ 0 ] ) * 6 );
		s = (( rgb[ 2 ] - rgb[ 0 ] ) * s_range ) / rgb[ 2 ];
		v = ( rgb[ 2 ] * v_range ) / rgb_range;
	}

#if COLOR_CONV_RANGE_CHECK	
	if ( h < 0 || h > rgb_range
		 || s < 0 || s > rgb_range
		 || v < 0 || v > rgb_range ) {
		std::cerr << "ColorConv<T>::RgbToHsv(): conversion out of range"
				  << std::endl;
		std::cerr << "(" << r << "," << g << "," << b << ")->" 
				  << "(" << h << "," << s << "," << v << ")" << std::endl;
	}
#endif	
}

template<class T>
void
ColorConv<T>::HsvToRgb( const T h0, const T s0, const T v0, T& r, T& g, T& b )
{
	double	h, s, v, w;

	h = h0 / static_cast<double>( h_range );
    if ( h < 0.0 ) {
        h += static_cast<int>( -h ) + 1;
    }
	if ( h >= 1.0 ) {
        h -= static_cast<int>( h );
    }
	h = h * 6.0;
	s = s0 / static_cast<double>( s_range );
	v = v0 / static_cast<double>( v_range );
	w = v * ( 1.0 - s );
	
	switch ( static_cast<int>( h )) {
	case 0:
		r = static_cast<T>( v * rgb_range );
		b = static_cast<T>( w * rgb_range );
		g = b + static_cast<T>( h * ( v - w ) * rgb_range );
		break;
	case 1:
		g = static_cast<T>( v * rgb_range );
		b = static_cast<T>( w * rgb_range );
		r = b - static_cast<T>(( h - 2.0 ) * ( v - w ) * rgb_range );
		break;
	case 2:
		g = static_cast<T>( v * rgb_range );
		r = static_cast<T>( w * rgb_range );
		b = r + static_cast<T>(( h - 2.0 ) * ( v - w ) * rgb_range );
		break;
	case 3:
		b = static_cast<T>( v * rgb_range );
		r = static_cast<T>( w * rgb_range );
		g = r - static_cast<T>(( h - 4.0 ) * ( v - w ) * rgb_range );
		break;
	case 4:
		b = static_cast<T>( v * rgb_range );
		g = static_cast<T>( w * rgb_range );
		r = g + static_cast<T>(( h - 4.0 ) * ( v - w ) * rgb_range );
		break;
	case 5:
		r = static_cast<T>( v * rgb_range );
		g = static_cast<T>( w * rgb_range );
		b = g - static_cast<T>(( h - 6.0 ) * ( v - w ) * rgb_range );
		break;
	default:
		r = g = b = 0;
		break;
	}

#if COLOR_CONV_RANGE_CHECK
	if ( r < 0 || r > rgb_range
		 || g < 0 || g > rgb_range
		 || b < 0 || b > rgb_range ) {
		std::cerr << "ColorConv<T>::HsvToRgb(): conversion out of range"
				  << std::endl;
		std::cerr << "(" << h << "," << s << "," << v << ")->"
				  << "(" << r << "," << g << "," << b << ")" << std::endl;
	}
#endif
}

#endif	// __COLOR_CONV_H__
