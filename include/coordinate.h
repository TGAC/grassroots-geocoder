/*
 * coordinate.h
 *
 *  Created on: 31 May 2018
 *      Author: billy
 */


#ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_
#define TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_

#include <math.h>

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

/**
 * A datatype for storing a geographic coordinate
 *
 * @ingroup geocoder_library
 */
typedef struct
{
	/**
	 *  The latitude, in degrees, which has a valid range from -90 and +90 for the southern and northern hemisphere respectively.
	 */
	double64 co_x;

	/**
	 * The longitude, in degrees, which has a valid range from -180 and +180 specifying coordinates west and east of the Prime Meridian, respectively.
	 *
	 */
	double64 co_y;

	/**
	 * A pointer to the elevation value, in metres, for this Coordinate.
	 * This can be <code>NULL</code> which means that the value has not
	 * been set for this Coordinate.
	 */
	double64 *co_elevation_p;
} Coordinate;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_COORDINATE_TAGS
	#define COORDINATE_PREFIX GRASSROOTS_GEOCODER_API
	#define COORDINATE_VAL(x)	= x
#else
	#define COORDINATE_PREFIX extern GRASSROOTS_GEOCODER_API
	#define COORDINATE_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



/**
 * The key for the latitude of for a location object for a given record.
 *
 * @ingroup geocoder_library
 */
COORDINATE_PREFIX const char *CO_LATITUDE_S COORDINATE_VAL ("latitude");


/**
 * The key for the longitude of for a location object for a given record.
 *
 * @ingroup geocoder_library
 */
COORDINATE_PREFIX const char *CO_LONGITUDE_S COORDINATE_VAL ("longitude");


/**
 * The key for the elevation of for a location object for a given record.
 *
 * @ingroup geocoder_library
 */
COORDINATE_PREFIX const char *CO_ELEVATION_S COORDINATE_VAL ("elevation");



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a coordinate for a given latitude and longitude.
 *
 * @param x The latitude to use.
 * @param y The longitude to use.
 * @return The new Coordinate or <code>NULL</code> upon error.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API Coordinate *AllocateCoordinate (double64 x, double64 y);


/**
 * Initialise a coordinate as 0,0
 *
 * @param coord_p The Coordinate to initialise.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API void InitCoordinate (Coordinate *coord_p);


/**
 * Free a Coordinate.
 *
 * @param coord_p The Coordinate to free.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API void FreeCoordinate (Coordinate *coord_p);


/**
 * Get the JSON representation of a Coordinate.
 *
 * @param coord_p The Coordinate to get the JSON representation for.
 * @return The JSON representation or <code>NULL</code> upon error.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API json_t *GetCoordinateAsJSON (const Coordinate * const coord_p);


/**
 * Add the JSON representation for a given Coordinate to a JSON fragment using a given key.
 *
 * The value added for the Coordinate is derived using SetCoordinateFromJSON().
 *
 * @param coord_p The Coordinate to add to the JSON fragment.
 * @param dest_p The JSON fragment to add the Coordinate's JSON to.
 * @param coord_key_s The key used to add the Coordinate's JSON to the given JSON fragment.
 * @return <code>true</code> if the Coordinate was added successfully, <code>false</code> otherwise.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool AddCoordinateToJSON (const Coordinate *coord_p, json_t *dest_p, const char * const coord_key_s);


/**
 * Set the Coordinate from a JSON representation.
 *
 * @param coord_p The Coordinate to set the values for.
 * @param The JSON representation to get the values from.
 * @return <code>true</code> if the Coordinate was set successfully, <code>false</code> otherwise.
 * @memberof Coordinate.
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool SetCoordinateFromJSON (Coordinate *coord_p, const json_t *value_p);



GRASSROOTS_GEOCODER_API bool SetCoordinateFromCompoundJSON (Coordinate *coord_p, const json_t *value_p, const char * const coord_key_s);


/**
 * Set the elevation for a given Coordinate.
 *
 * @param coord_p The Coordinate to set the elevation for.
 * @param elevation The elevation value to set for the given Coordinate.
 * @return <code>true</code> if the Coordinate's elevation was set successfully, <code>false</code> otherwise.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool SetCoordinateElevation (Coordinate *coord_p, double64 elevation);


/**
 * Clear the elevation for a given Coordinate.
 *
 * @param coord_p The Coordinate to clear the elevation for.
 * @memberof Coordinate
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API void ClearCoordinateElevation (Coordinate *coord_p);


#ifdef __cplusplus
}
#endif



#endif		/* #ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_COORD_H_ */
