/*
** Copyright 2014-2018 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * nominatim.c
 *
 *  Created on: 15 Aug 2019
 *      Author: billy
 */

#include <ctype.h>

#include "nominatim.h"

#include "streams.h"
#include "math_utils.h"
#include "json_util.h"
#include "byte_buffer.h"
#include "curl_tools.h"
#include "string_utils.h"
#include "math_utils.h"
#include "geocoder_util.h"



static int PopulateAddressForNominatim (Address *address_p, const json_t *result_p);

static bool PopulateGPSForNominatim (Address *address_p, const json_t *result_p);



static bool GetDoubleValue (const json_t *json_p, const char *key_s, double *value_p);
static bool SetValidAddressComponent (const json_t *json_p, const char *key_s, char **value_ss);





bool RunNominatimGeocoder (Address *address_p, const char *geocoder_uri_s)
{
	bool got_location_flag = false;



	return got_location_flag;
}




bool RunNominatimReverseGeocoder (Address *address_p, const char *reverse_geocoder_url_s)
{
	bool got_location_flag = false;

	if (address_p -> ad_gps_centre_p)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					if (AppendStringToByteBuffer (buffer_p, reverse_geocoder_url_s))
						{
							char *lat_s = ConvertDoubleToString (address_p -> ad_gps_centre_p -> co_x);

							if (lat_s)
								{
									if (AppendStringsToByteBuffer (buffer_p, "?format=json&lat=", lat_s, NULL))
										{
											char *lon_s = ConvertDoubleToString (address_p -> ad_gps_centre_p -> co_y);

											if (lon_s)
												{
													if (AppendStringsToByteBuffer (buffer_p, "&lon=", lon_s, NULL))
														{
															CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

															if (curl_p)
																{
																	const char *url_s = GetByteBufferData (buffer_p);

																	if (CallGeocoderWebService (curl_p, url_s, address_p, PopulateAddressForNominatim))
																		{
																			got_location_flag = true;
																		}

																	FreeCurlTool (curl_p);
																}
														}

													FreeCopiedString (lon_s);
												}
										}

									FreeCopiedString (lat_s);
								}
						}

					FreeByteBuffer (buffer_p);
				}		/* if (buffer_p) */

		}		/* if (address_p -> ad_gps_centre_p) */



	return got_location_flag;
}




/*
  {
    "place_id": "100149",
    "licence": "Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright",
    "osm_type": "node",
    "osm_id": "107775",
    "boundingbox": ["51.3473219", "51.6673219", "-0.2876474", "0.0323526"],
    "lat": "51.5073219",
    "lon": "-0.1276474",
    "display_name": "London, Greater London, England, SW1A 2DU, United Kingdom",
    "class": "place",
    "type": "city",
    "importance": 0.9654895765402,
    "icon": "https://nominatim.openstreetmap.org/images/mapicons/poi_place_city.p.20.png",
    "address": {
      "city": "London",
      "state_district": "Greater London",
      "state": "England",
      "postcode": "SW1A 2DU",
      "country": "United Kingdom",
      "country_code": "gb"
    },
    "extratags": {
      "capital": "yes",
      "website": "http://www.london.gov.uk",
      "wikidata": "Q84",
      "wikipedia": "en:London",
      "population": "8416535"
    }
  },
 */
static int PopulateAddressForNominatim (Address *address_p, const json_t *result_p)
{
	bool success_flag = false;
	const json_t *address_json_p = json_object_get (result_p, "address");

	if (address_json_p)
		{
			if (SetValidAddressComponent (address_json_p, "city", & (address_p -> ad_town_s)))
				{
					if (SetValidAddressComponent (address_json_p, "state_district", & (address_p -> ad_county_s)))
						{
							if (SetValidAddressComponent (address_json_p, "country", & (address_p -> ad_country_s)))
								{
									if (SetValidAddressComponent (address_json_p, "country_code", & (address_p -> ad_country_code_s)))
										{
											if (SetValidAddressComponent (address_json_p, "postcode", & (address_p -> ad_postcode_s)))
												{
													success_flag = true;
												}		/* if (SetValidAddressComponent (address_json_p, "postcode", & (address_p -> ad_postcode_s))) */

										}		/* if (SetValidAddressComponent (address_json_p, "country_code", & (address_p -> ad_country_code_s))) */

								}		/* if (SetValidAddressComponent (address_json_p, "country", & (address_p -> ad_country_s))) */

						}		/* if (SetValidAddressComponent (address_json_p, "state_district", & (address_p -> ad_county_s))) */

				}		/* if (SetValidAddressComponent (address_json_p, "city", & (address_p -> ad_town_s))) */

		}		/* if (address_json_p) */


	return success_flag;
}



static bool PopulateGPSForNominatim (Address *address_p, const json_t *result_p)
{
	bool success_flag = false;
	double latitude = 0.0;

	if (GetDoubleValue (result_p, "lat", &latitude))
		{
			double longitude = 0.0;

			if (GetDoubleValue (result_p, "lon", &longitude))
				{
					if (SetAddressCentreCoordinate (address_p, latitude, longitude, NULL))
						{
							/*
							 * We've set the main coordinate,
							 * let's see if we can do the bounds
							 */
							const json_t *bounds_p = json_object_get (result_p, "boundingbox");

							if (bounds_p)
								{
									if ((json_is_array (bounds_p)) && (json_array_size (bounds_p) == 4))
										{
											/*
											 * The order of the coords are left, right, bottom, top
											 */
											double east = 0.0;
											json_t *entry_p = json_array_get (bounds_p, 0);
											const char *value_s = json_string_value (entry_p);

											if (GetValidRealNumber (&value_s, &east, NULL))
												{
													double west = 0.0;

													entry_p = json_array_get (bounds_p, 1);
													value_s = json_string_value (entry_p);

													if (GetValidRealNumber (&value_s, &west, NULL))
														{
															double south = 0.0;

															entry_p = json_array_get (bounds_p, 2);
															value_s = json_string_value (entry_p);

															if (GetValidRealNumber (&value_s, &south, NULL))
																{
																	double north = 0.0;

																	entry_p = json_array_get (bounds_p, 3);
																	value_s = json_string_value (entry_p);

																	if (GetValidRealNumber (&value_s, &north, NULL))
																		{
																			if (!SetAddressNorthEastCoordinate (address_p, east, north, NULL))
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set NE coord to %lf, %lf", east, north);
																				}

																			if (!SetAddressSouthWestCoordinate (address_p, west, south, NULL))
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set SW coord to %lf, %lf", west, south);
																				}

																		}		/* if (GetValidRealNumber (json_array_get (bounds_p, 3), &north, NULL)) */

																}		/* if (GetValidRealNumber (json_array_get (bounds_p, 2), &south, NULL)) */

														}		/* if (GetValidRealNumber (json_array_get (bounds_p, 1), &west, NULL)) */

												}		/* if (GetValidRealNumber (json_array_get (bounds_p, 0), &east, NULL)) */

										}		/* if ((json_is_array (bounds_p)) && (json_array_size (bounds_p) == 4)) */

								}		/* if (bounds_p) */


							success_flag = true;
						}		/* if (SetAddressCentreCoordinate (address_p, latitude, longitude, NULL)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set centre coord to %lf, %lf", latitude, longitude);
						}

				}		/* if (GetDoubleValue (result_p, "lon", &longitude)) */

		}		/* if (GetDoubleValue (result_p, "lat", &latitude)) */

	return success_flag;
}


static bool SetValidAddressComponent (const json_t *json_p, const char *key_s, char **value_ss)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (json_p, key_s);

	if (value_s)
		{
			char *copied_value_s = EasyCopyToNewString (value_s);

			if (copied_value_s)
				{
					if (*value_ss)
						{
							FreeCopiedString (*value_ss);
						}

					*value_ss = copied_value_s;
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to copy \"%s\" for key \"%s\"", value_s, key_s);
				}

		}		/* if (value_s) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool GetDoubleValue (const json_t *json_p, const char *key_s, double *value_p)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (json_p, key_s);

	if (value_s)
		{
			if (GetValidRealNumber (&value_s, value_p, NULL))
				{
					success_flag = true;
				}
		}

	return success_flag;
}
