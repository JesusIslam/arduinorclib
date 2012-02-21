/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** util.cpp
** Utility functions
**
** Author: Daniel van den Ouden
** Project: ArduinoRCLib
** Website: http://sourceforge.net/p/arduinorclib/
** -------------------------------------------------------------------------*/

#include <rc/util.h>


namespace rc
{

static uint16_t s_center = 1520;
static uint16_t s_travel = 600;

int16_t microsToNormalized(uint16_t p_micros)
{
	// first we clip values, early abort.
	if (p_micros >= s_center + s_travel)
	{
		return 256;
	}
	else if (p_micros <= s_center - s_travel)
	{
		return -256;
	}
	
	// get the absolute delta ABS(p_micros - s_center)
	uint16_t delta = (p_micros > s_center) ? (p_micros - s_center) : (s_center - p_micros);
	
	// we need to multiply by end range 256 and divide by start range m_travel
	// and we need to do this without risking overflows...
	
	// The max value in delta will be s_travel, about 1000. This gives us 6 bits of room to play with
	// So instead of multiplying with 256 and dividing by s_travel,
	// we multiply by 64 and divide by s_travel / 4
	// we lose the last two bits of the division, but that's not going to make much of a difference...
	delta <<= 6;
	delta /= (s_travel >> 2);
	
	return (p_micros >= s_center) ? delta : -delta;
}


uint16_t normalizedToMicros(int16_t p_normal)
{
	// we have a normalized value [-256 - 256] which corresponds to full positive or negative servo movement
	// we need to scale this to a [0 - 2 * s_travel] microseconds range
	// this needs to be done in multiple stages to prevent overflows while keeping maximum resolution
	p_normal += 256; // first we bring it up to [0 - 512]
	
	// calculate range
	uint16_t range = s_travel << 1;
	
	// we assume the range won't be more than 2000 microseconds, 2 milliseconds
	// this means we have 5 bits of room to play with
	// we can safely multiply any 5 bit number with the range without having overflows
	// the normalized value is 10 bits in size
	// we will chop it up into two pieces of 5 bits
	// then we will do the multiplications, divide the results by 512, and shift the results back
	// then we'll piece the results back together to form the end result
	
	// cut  into pieces
	uint16_t p1 =  p_normal       & 0x1F; // bits 0-4
	uint16_t p2 = (p_normal >> 5) & 0x1F; // bits 5-9
	
	// multiply by end range
	p1 *= range;
	p2 *= range;
	
	// divide by start range, shift back
	p1 >>= 9; // divide by 512
	p2 >>= 4; // divide by 16; ( / 512) << 5
	
	// piece it back together, offset with center
	return ((s_center - s_travel) + p1 + p2);
}


void setCenter(uint16_t p_center)
{
	s_center = p_center;
}


uint16_t getCenter()
{
	return s_center;
}


void setTravel(uint16_t p_travel)
{
	s_travel = p_travel;
}


uint16_t getTravel()
{
	return s_travel;
}


void loadFutaba()
{
	setCenter(1520);
	setTravel(600);
}


void loadJR()
{
	setCenter(1500);
	setTravel(600);
}


// namespace end
}