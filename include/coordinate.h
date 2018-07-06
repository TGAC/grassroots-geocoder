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



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_COORDINATE_TAGS
	#define COORDINATE_PREFIX GRASSROOTS_GEOCODER_LOCAL
	#define COORDINATE_VAL(x)	= x
#else
	#define COORDINATE_PREFIX extern
	#define COORDINATE_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



/**
 * The key for the latitude of for a location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
COORDINATE_PREFIX const char *CO_LATITUDE_S COORDINATE_VAL ("latitude");


/**
 * The key for the longitude of for a location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
COORDINATE_PREFIX const char *CO_LONGITUDE_S COORDINATE_VAL ("longitude");



#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_GEOCODER_API Coordinate *AllocateCoordinate (double64 x, double64 y);


GRASSROOTS_GEOCODER_API void FreeCoordinate (Coordinate *coord_p);


GRASSROOTS_GEOCODER_API json_t *GetCoordinateAsJSON (const Coordinate * const coord_p);


GRASSROOTS_GEOCODER_API bool AddCoordinateToJSON (const Coordinate *coord_p, json_t *dest_p, const char * const coord_key_s);


GRASSROOTS_GEOCODER_API bool SetCoordinateFromJSON (Coordinate *coord_p, const json_t *value_p);

#ifdef __cplusplus
}
#endif



#endif		/* #ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_ */
