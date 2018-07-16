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

/**
 * A datatype for storing a postal address as well
 * as its geographic coordinates if possible.
 *
 * @ingroup geocoder_library
 */
typedef struct Address
{
	/**
	 * The building name or number for this Address.
	 *
	 */
	char *ad_name_s;

	/**
	 * The street that this Address is on.
	 */
	char *ad_street_s;


	/**
	 * The town, city or village that this Address is in.
	 */
	char *ad_town_s;


	/**
	 * The county that this Address is in.
	 */
	char *ad_county_s;

	/**
	 * The country that this Address is in.
	 */
	char *ad_country_s;

	/**
	 * The postal code for this Address.
	 */
	char *ad_postcode_s;

	/**
	 * The ISO 3166-1 alpha-2 country code for the country that this
	 * Address is in.
	 */
	char *ad_country_code_s;

	/**
	 * The string representation of the geographic coordinate for this
	 * Address such as "32.4567, 12.1234" for a latitude of 32.4567 degrees
	 * and a longitude of 12.1234 degrees. This can be <code>NULL</code> and
	 * the value from Address::ad_gps_centre_p should always be used, where possible,
	 * instead of this.
	 */
	char *ad_gps_s;

	/**
	 * A pointer to the Coordinate for the central point of this Address.
	 * This can be <code>NULL</code> if it has not yet been calculated
	 */
	Coordinate *ad_gps_centre_p;


	/**
	 * A pointer to the Coordinate for the north-east bounds of this Address.
	 * This can be <code>NULL</code> if it has not yet been calculated
	 */
	Coordinate *ad_gps_north_east_p;


	/**
	 * A pointer to the Coordinate for the south-west bounds of this Address.
	 * This can be <code>NULL</code> if it has not yet been calculated
	 */
	Coordinate *ad_gps_south_west_p;
} Address;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_ADDRESS_TAGS
	#define ADDRESS_PREFIX GRASSROOTS_GEOCODER_API
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
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_LOCATION_S ADDRESS_VAL ("location");


/**
 * The key for the north-eastern bounds for the location object for a given record.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_NORTH_EAST_LOCATION_S ADDRESS_VAL ("north_east_bound");


/**
 * The key for the south-western bounds for the location object for a given record.
 *
 * @ingroup geocoder_library
 */
ADDRESS_PREFIX const char *AD_SOUTH_WEST_LOCATION_S ADDRESS_VAL ("south_west_bound");


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocte a new Address object.
 *
 * @param name_s The building name or number the new Address.
 * @param street_s The street for the new Address.
 * @param town_s The street for the new Address.
 * @param county_s The town, city or village that the new Address is in.
 * @param country_s The country that the new Address is in.
 * @param postcode_s The postal code for the new Address.
 * @param country_code_s The ISO 3166-1 alpha-2 country code for the country that this new Address is in.
 * @param gps_s The string representation of the geographic coordinate for this
 * Address such as "32.4567, 12.1234" for a latitude of 32.4567 degrees
 * and a longitude of 12.1234 degrees. This can be <code>NULL</code>
 * @return The newly-allocated Address or <code>NULL</code> upon error.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API Address *AllocateAddress (const char *name_s, const char *street_s, const char *town_s, const char *county_s, const char *country_s, const char *postcode_s, const char *country_code_s, const char *gps_s);


/**
 * Free an Address.
 *
 * @param address_p The Address to free.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API void FreeAddress (Address *address_p);


/**
 * Clear all of the values within an Address.
 *
 * @param address_p The Address to clear.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API void ClearAddress (Address *address_p);


/**
 * Get the JSON representation of an Address.
 *
 * This calls ConvertAddressToJSON() on a newly-allocated JSON object.
 *
 * @param address_p The Address to get the JSON representation for.
 * @return The JSON representation or <code>NULL</code> upon error.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API json_t *GetAddressAsJSON (const Address *address_p);

/**
 * Store the JSON-based representation of an Address in a given JSON fragment.
 *
 * @param address_p The address to get the value from.
 * @param dest_p The JSON fragment where the values for the given Address will be stored.
 * @return <code>true</code> if the Address was converted successfully, <code>false</code> otherwise.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool ConvertAddressToJSON (const Address *address_p, json_t *dest_p);


/**
 * Populate a JSON fragment with the values from a given Address conforming to the definition of
 * a postal address from schema.org given at https://schema.org/PostalAddress.
 *
 * @param address_p The Address to get the values from.
 * @param values_p The JSON fragment to add the JSON-based representation of the given Address to
 * @param address_key_s The key that will be used in values_p to add the JSON-based representation for.
 * @return <code>true</code> if the details for the Address were added successfully, <code>false</code> otherwise.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool ParseAddressForSchemaOrg (const Address *address_p, json_t *values_p, const char *address_key_s);


/**
 * Set the Coordinate for the central point of a given address to the given
 * latitude and longitude values.
 *
 * @param address_p The Address to set the central point for.
 * @param latitude The latitude, in degrees, of the central point to set for the given Address.
 * @param longitude The longitude, in degrees, of the central point to set for the given Address.
 * @return <code>true</code> if the Coordinate was set successfully, <code>false</code> otherwise.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool SetAddressCentreCoordinate (Address *address_p, const double64 latitude, const double64 longitude);


/**
 * Set the Coordinate for the north-east bounds of a given address to the given
 * latitude and longitude values.
 *
 * @param address_p The Address to set the south-west bounds for.
 * @param latitude The latitude, in degrees, of the north-east bounds to set for the given Address.
 * @param longitude The longitude, in degrees, of the north-east bounds to set for the given Address.
 * @return <code>true</code> if the Coordinate was set successfully, <code>false</code> otherwise.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool SetAddressNorthEastCoordinate (Address *address_p, const double64 latitude, const double64 longitude);


/**
 * Set the Coordinate for the south-west bounds of a given address to the given
 * latitude and longitude values.
 *
 * @param address_p The Address to set the south-west bounds for.
 * @param latitude The latitude, in degrees, of the south-west bounds to set for the given Address.
 * @param longitude The longitude, in degrees, of the south-west bounds to set for the given Address.
 * @return <code>true</code> if the Coordinate was set successfully, <code>false</code> otherwise.
 * @memberof Address
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool SetAddressSouthWestCoordinate (Address *address_p, const double64 latitude, const double64 longitude);



#ifdef __cplusplus
}
#endif

#endif /* TOOLS_GRASSROOTS_GEOCODER_INCLUDE_ADDRESS_H_ */
