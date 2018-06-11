/*
 * address.h
 *
 *  Created on: 31 May 2018
 *      Author: billy
 */

#ifndef TOOLS_GRASSROOTS_GEOCODER_INCLUDE_ADDRESS_H_
#define TOOLS_GRASSROOTS_GEOCODER_INCLUDE_ADDRESS_H_

#include "grassroots_geocoder_library.h"
#include "coordinate.h"
#include "jansson.h"

typedef struct Address
{
	char *ad_name_s;
	char *ad_street_s;
	char *ad_town_s;
	char *ad_county_s;
	char *ad_country_s;
	char *ad_postcode_s;
	char *ad_country_code_s;
	char *ad_gps_s;
	Coordinate *ad_gps_centre_p;
	Coordinate *ad_gps_north_east_p;
	Coordinate *ad_gps_south_west_p;
} Address;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_GEOCODER_TAGS
	#define ADDRESS_PREFIX GRASSROOTS_GEOCODER_LOCAL
	#define ADDRESS_VAL(x)	= x
#else
	#define ADDRESS_PREFIX extern
	#define ADDRESS_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


ADDRESS_PREFIX const char *AD_ADDRESS_S ADDRESS_VAL ("Address");

/**
 * The key for specifying an object containing the GPS coordinates
 * where the sample was collected.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_GPS_S ADDRESS_VAL ("GPS");


/**
 * The key for specifying an object containing the postal code
 * where the sample was collected.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_POSTCODE_S ADDRESS_VAL ("Postal code");


/**
 * The key for specifying an object containing the street address
 * where the sample was collected. For example the house number and
 * street name.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_STREET_S ADDRESS_VAL ("Street");

/**
 * The key for specifying an object containing the town
 * where the sample was collected.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_TOWN_S ADDRESS_VAL ("Town");


/**
 * The key for specifying an object containing the county
 * where the sample was collected.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_COUNTY_S ADDRESS_VAL ("County");


/**
 * The key for specifying an object containing the country
 * where the sample was collected.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_COUNTRY_S ADDRESS_VAL ("Country");




/**
 * The key for the location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
ADDRESS_PREFIX const char *AD_LOCATION_S ADDRESS_VAL ("location");


/**
 * The key for the latitude of for a location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
ADDRESS_PREFIX const char *AD_LATITUDE_S ADDRESS_VAL ("latitude");


/**
 * The key for the longitude of for a location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
ADDRESS_PREFIX const char *AD_LONGITUDE_S ADDRESS_VAL ("longitude");


/**
 * The key for the north-eastern bounds for the location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
ADDRESS_PREFIX const char *AD_NORTH_EAST_LOCATION_S ADDRESS_VAL ("north_east_bound");


/**
 * The key for the south-western bounds for the location object for a given record.
 *
 * @ingroup pathogenomics_service
 */
ADDRESS_PREFIX const char *AD_SOUTH_WEST_LOCATION_S ADDRESS_VAL ("south_west_bound");


#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_GEOCODER_API Address *AllocateAddress (const char *name_s, const char *street_s, const char *town_s, const char *county_s, const char *country_s, const char *postcode_s, const char *country_code_s, const char *gps_s);


GRASSROOTS_GEOCODER_API void FreeAddress (Address *address_p);


GRASSROOTS_GEOCODER_API void ClearAddress (Address *address_p);


GRASSROOTS_GEOCODER_API json_t *GetAddressAsJSON (const Address *address_p);


GRASSROOTS_GEOCODER_API bool ConvertAddressToJSON (const Address *address_p, json_t *dest_p);


GRASSROOTS_GEOCODER_API bool ParseAddressForSchemaOrg (const Address *address_p, json_t *values_p, const char *address_key_s);


GRASSROOTS_GEOCODER_API bool SetAddressCentreCoordinate (Address *address_p, const double64 latitude, const double64 longitude);


GRASSROOTS_GEOCODER_API bool SetAddressNorthEastCoordinate (Address *address_p, const double64 latitude, const double64 longitude);


GRASSROOTS_GEOCODER_API bool SetAddressSouthWestCoordinate (Address *address_p, const double64 latitude, const double64 longitude);



#ifdef __cplusplus
}
#endif

#endif /* TOOLS_GRASSROOTS_GEOCODER_INCLUDE_ADDRESS_H_ */
