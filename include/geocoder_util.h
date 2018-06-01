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


typedef struct GeocoderConfiguration
{

} GeocoderConfiguration;


#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_GEOCODER_API bool DetermineGPSLocationForAddress (Address *address_p);


GRASSROOTS_GEOCODER_LOCAL bool DetermineGPSLocationForAddressByGoogle (Address *address_p, const char *geocoder_uri_s);

GRASSROOTS_GEOCODER_LOCAL bool DetermineGPSLocationForAddressByOpencage (Address *address_p, const char *geocoder_uri_s);


GRASSROOTS_GEOCODER_LOCAL bool RefineLocationDataForGoogle (Address *address_p, const json_t *raw_data_p);


GRASSROOTS_GEOCODER_LOCAL bool FillInAddressFromGoogleData (Address *address_p, const json_t *google_result_p);




#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_UTIL_INCLUDE_GEOCODER_UTIL_H_ */
