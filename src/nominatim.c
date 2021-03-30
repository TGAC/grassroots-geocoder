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

static int ParseNominatimResults (Address *address_p, const json_t *web_service_results_p);

static bool SetValidAddressComponent (const json_t *json_p, const char *key_s, char **value_ss);

static int AddEscapedValue (ByteBuffer *buffer_p, const char *key_s, const char *value_s, bool *first_param_flag_p, CurlTool *tool_p);

static bool BuildNominatimURLUsingComponentsParameters (ByteBuffer *buffer_p, const Address * const address_p, CurlTool *tool_p);


bool RunNominatimGeocoder (Address *address_p, const char *geocoder_uri_s)
{
	bool got_location_flag = false;

	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
				{
					CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

					if (curl_p)
						{
							if (BuildNominatimURLUsingComponentsParameters (buffer_p, address_p, curl_p))
								{
									const char *url_s = GetByteBufferData (buffer_p);
									int res = CallGeocoderWebService (curl_p, url_s, address_p, ParseNominatimResults);

									if (res == 1)
										{
											got_location_flag = true;
										}
									else if (res == 0)
										{
											ResetByteBuffer (buffer_p);

											if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
												{
													if (BuildURLUsingAddressParameter (buffer_p, curl_p, address_p, "&q=", ",%20"))
														{
															url_s = GetByteBufferData (buffer_p);
															res = CallGeocoderWebService (curl_p, url_s, address_p, ParseNominatimResults);

															if (res == 1)
																{
																	got_location_flag = true;
																}
														}
												}

										}

								}

							FreeCurlTool (curl_p);
						}



				}		/* if (AppendStringToByteBuffer (buffer_p, data_p -> msd_geocoding_uri_s)) */
			else
				{
				}

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

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
															if (AppendStringToByteBuffer (buffer_p, "&addressdetails=1"))
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
			if (SetValidAddressComponent (address_json_p, "street", & (address_p -> ad_street_s)))
				{
					if (SetValidAddressComponent (address_json_p, "city", & (address_p -> ad_town_s)))
						{
							if (SetValidAddressComponent (address_json_p, "county", & (address_p -> ad_county_s)))
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

				}		/* if (SetValidAddressComponent (address_json_p, "street", & (address_p -> ad_street_s))) */

		}		/* if (address_json_p) */


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


/*
 [
  {
    "place_id": 83454990,
    "licence": "Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright",
    "osm_type": "way",
    "osm_id": 10055537,
    "boundingbox": [
      "52.0904469",
      "52.1077327",
      "-1.2895808",
      "-1.2849026"
    ],
    "lat": "52.0999714",
    "lon": "-1.2889324",
    "display_name": "Wardington Road, Wardington, Cherwell, Oxfordshire, East Midlands, England, OX17 1FE, United Kingdom",
    "class": "highway",
    "type": "unclassified",
    "importance": 0.51
  },
  {
    "place_id": 80632782,
    "licence": "Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright",
    "osm_type": "way",
    "osm_id": 3979056,
    "boundingbox": [
      "52.1077327",
      "52.1082139",
      "-1.2850519",
      "-1.2844854"
    ],
    "lat": "52.1080916",
    "lon": "-1.2848678",
    "display_name": "Wardington Road, Wardington, Cherwell, Oxfordshire, South East, England, OX17 1FE, United Kingdom",
    "class": "highway",
    "type": "unclassified",
    "importance": 0.51
  }
]
 *
 */
static int ParseNominatimResults (Address *address_p, const json_t *web_service_results_p)
{
	int res = -1;

	if (web_service_results_p)
		{
			if (json_is_array (web_service_results_p))
				{
					const size_t num_results = json_array_size (web_service_results_p);

					if (num_results > 0)
						{
							size_t i = 0;

							while ((i < num_results) && (res == -1))
								{
									const json_t *result_p = json_array_get (web_service_results_p, i);
									double latitude;

									if (GetJSONStringAsDouble (result_p, "lat", &latitude))
										{
											double longitude;

											if (GetJSONStringAsDouble (result_p, "lon", &longitude))
												{
													if (SetAddressCentreCoordinate (address_p, latitude, longitude, NULL))
														{
															const json_t *bounds_p = json_object_get (result_p, "boundingbox");

															if (bounds_p)
																{
																	if (json_is_array (bounds_p))
																		{
																			size_t num_bounds = json_array_size (bounds_p);

																			if (num_bounds == 4)
																				{
																					double min_latitude;

																					if (GetRealValueFromJSONString (json_array_get (bounds_p, 0), &min_latitude))
																						{
																							double max_latitude;

																							if (GetRealValueFromJSONString (json_array_get (bounds_p, 1), &max_latitude))
																								{
																									double min_longitude;

																									if (GetRealValueFromJSONString (json_array_get (bounds_p, 2), &min_longitude))
																										{
																											double max_longitude;

																											if (GetRealValueFromJSONString (json_array_get (bounds_p, 3), &max_longitude))
																												{
																													if (SetAddressNorthEastCoordinate (address_p, min_latitude, min_longitude, NULL))
																														{
																															if (SetAddressSouthWestCoordinate (address_p, max_latitude, max_longitude, NULL))
																																{
																																	res = 1;
																																}
																															else
																																{
																																	PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set south-west location from %lf %lf ", max_latitude, max_longitude);
																																}
																														}
																													else
																														{
																															PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set north-east location from %lf %lf ", min_latitude, min_longitude);
																														}

																												}		/* if (GetRealValueFromJSONString (json_array_get (bounds_p, 3), &max_longitude)) */
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, bounds_p, "failed to get index 3 as real value from string");
																												}

																										}		/* if (GetRealValueFromJSONString (json_array_get (bounds_p, 2), &min_longitude)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, bounds_p, "failed to get index 2 as real value from string");
																										}

																								}		/* if (GetRealValueFromJSONString (json_array_get (bounds_p, 1), &max_latitude)) */
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, bounds_p, "failed to get index 1 as real value from string");
																								}

																						}		/* if (GetRealValueFromJSONString (json_array_get (bounds_p, 0), &min_latitude)) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, bounds_p, "failed to get index 0 as real value from string");
																						}

																				}		/* if (num_bounds == 4) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "boundingbox doesn't have 4 entries");
																				}

																		}		/* if (json_is_array (bounds_p)) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "boundingbox is not an array");
																		}

																}		/* if (bounds_p) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to get bounds");
																}

														}		/* if (SetAddressCentreCoordinate (address_p, latitude, longitude, NULL)) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetAddressCentreCoordinate failed for \"%s\" to %lf,%lf", address_p -> ad_name_s, latitude, longitude);
														}

												}		/* if (GetJSONStringAsDouble (result_p, "lon", &longitude)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to get longitude");
												}

										}		/* if (GetJSONStringAsDouble (result_p, "lat", &latitude)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to get latitude");
										}

									++ i;
								}		/* while ((i < num_results) && (res == -1)) */
						}
					else
						{
							res = 0;
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, web_service_results_p, "response is not an array");
				}

		}
	else
		{
			res = 0;
		}

	return res;
}



static bool BuildNominatimURLUsingComponentsParameters (ByteBuffer *buffer_p, const Address * const address_p, CurlTool *tool_p)
{
	bool success_flag = false;
	bool first_param_flag = false;
	int res = AddEscapedValue (buffer_p, "street", address_p -> ad_street_s, &first_param_flag, tool_p);

	if (res != -1)
		{
			res = AddEscapedValue (buffer_p, "city", address_p -> ad_town_s, &first_param_flag, tool_p);

			if (res != -1)
				{
					res = AddEscapedValue (buffer_p, "county", address_p -> ad_county_s, &first_param_flag, tool_p);

					if (res != -1)
						{
							res = AddEscapedValue (buffer_p, "country", address_p -> ad_country_code_s, &first_param_flag, tool_p);

							if (res != -1)
								{
									res = AddEscapedValue (buffer_p, "postalcode", address_p -> ad_postcode_s, &first_param_flag, tool_p);

									if (res != -1)
										{
											success_flag = true;
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AddEscapedValue failed for country \"%s\"", address_p -> ad_country_code_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AddEscapedValue failed for county \"%s\"", address_p -> ad_county_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AddEscapedValue failed for city \"%s\"", address_p -> ad_town_s);
				}


		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AddEscapedValue failed for street \"%s\"", address_p -> ad_street_s);
		}



	return success_flag;
}


static int AddEscapedValue (ByteBuffer *buffer_p, const char *key_s, const char *value_s, bool *first_param_flag_p, CurlTool *tool_p)
{
	int res = 0;

	if (value_s)
		{
			char *escaped_value_s = NULL;

			/* remove any leading spaces */
			while ((*value_s != '\0') && (isspace (*value_s)))
				{
					++ value_s;
				}

			if (*value_s != '\0')
				{
					escaped_value_s = GetURLEscapedString (tool_p, value_s);

					if (escaped_value_s)
						{
							if (AppendStringsToByteBuffer (buffer_p, (*first_param_flag_p) ? "?": "&", key_s, "=", escaped_value_s, NULL))
								{
									res = 1;
									*first_param_flag_p = false;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AppendStringsToByteBuffer failed for \"%s\" and \"%s\"", key_s, escaped_value_s);
									res = -1;
								}

							FreeURLEscapedString (escaped_value_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetURLEscapedString failed for \"%s\"", value_s);
							res = -1;
						}
				}
		}

	return res;
}



