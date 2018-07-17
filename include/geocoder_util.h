/*
 * geocoder_util.h
 *
 *  Created on: 30 May 2018
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_GEOCODER_UTIL_H_
#define CORE_SHARED_UTIL_INCLUDE_GEOCODER_UTIL_H_

#include "grassroots_geocoder_library.h"
#include "typedefs.h"
#include "jansson.h"
#include "address.h"


/**
 * A datatype that accesses an external geocoding provider
 * to calculate the geographic data for an Address.
 *
 * This is generated automatically depending upon the configuration
 * options set in the geocoder Grassroots configuration file.
 * For further information,
 * please refer to the @ref geocoder_library documentation.
 *
 *
 * @ingroup geocoder_library
 */
typedef struct
{
	/**
	 * @private
	 * @param address_p
	 * @param uri_s
	 * @return
	 */
	bool (*gt_callback_fn) (Address *address_p, const char *uri_s);

	/**
	 * This is the URL of the geocoder service to use.
	 *
	 * This is set automatically depending upon the configuration
	 * options set in the geocoder configuration file.
	 * For further information,
	 * please refer to the @ref geocoder_library documentation.
	 *
	 * @private
	 */
	const char *gt_geocoder_uri_s;
} GeocoderTool;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Determine the geographic coordinates for a given Address using a given GeocoderTool.
 *
 * @param address_p The Address to determine the GPS coordinates for.
 * @param tool_p The GeocoderTool used to calculate the GPS coordinates for the given Address
 * @return <code>true</code> if the GPS location was calculated successfully, <code>false</code> otherwise.
 * @ingroup geocoder_library
 */
GRASSROOTS_GEOCODER_API bool DetermineGPSLocationForAddress (Address *address_p, GeocoderTool *tool_p);


GRASSROOTS_GEOCODER_LOCAL bool DetermineGPSLocationForAddressByGoogle (Address *address_p, const char *geocoder_uri_s);

GRASSROOTS_GEOCODER_LOCAL bool DetermineGPSLocationForAddressByOpencage (Address *address_p, const char *geocoder_uri_s);


GRASSROOTS_GEOCODER_LOCAL bool RefineLocationDataForGoogle (Address *address_p, const json_t *raw_data_p);


GRASSROOTS_GEOCODER_LOCAL bool FillInAddressFromGoogleData (Address *address_p, const json_t *google_result_p);




#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_UTIL_INCLUDE_GEOCODER_UTIL_H_ */
