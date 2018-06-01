/*
 * coordinate.h
 *
 *  Created on: 31 May 2018
 *      Author: billy
 */


#ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_
#define TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_



#include "grassroots_geocoder_library.h"
#include "typedefs.h"
#include "jansson.h"


/*
 * Taken from https://stackoverflow.com/questions/15965166/what-is-the-maximum-length-of-latitude-and-longitude
 *
 * The valid range of latitude in degrees is -90 and +90 for the southern and northern hemisphere respectively.
 * Longitude is in the range -180 and +180 specifying coordinates west and east of the Prime Meridian, respectively.
 *
 * So our value being outside this range means that the coordinate value hasn't been set
 */
#define COORD_UNSET (-1000.0)

typedef struct
{
	double64 po_x;
	double64 po_y;
} Coordinate;


#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_GEOCODER_API Coordinate *AllocateCoordinate (double64 x, double64 y);


GRASSROOTS_GEOCODER_API void FreeCoordinate (Coordinate *coord_p);


GRASSROOTS_GEOCODER_API json_t *GetCoordinateAsJSON (const Coordinate * const coord_p);


GRASSROOTS_GEOCODER_API bool AddCoordinateToJSON (const Coordinate *coord_p, json_t *dest_p, const char * const coord_key_s);


#ifdef __cplusplus
}
#endif



#endif		/* #ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_ */
