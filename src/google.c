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
 * google.c
 *
 *  Created on: 15 Aug 2019
 *      Author: billy
 */

#include <ctype.h>
#include <string.h>


#include "google.h"

#include "streams.h"
#include "math_utils.h"
#include "json_util.h"
#include "byte_buffer.h"
#include "country_codes.h"
#include "curl_tools.h"
#include "geocoder_util.h"

static int ParseGoogleResults (Address *address_p, const json_t *web_service_results_p);

static bool RefineLocationDataForGoogle (Address *address_p, const json_t *raw_data_p);

static bool FillInAddressFromGoogleData (Address *address_p, const json_t *google_result_p);

static bool BuildGoogleURLUsingAddressParameter (ByteBuffer *buffer_p, CurlTool *curl_p, const Address * const address_p);

static bool BuildGoogleURLUsingComponentsParameters (ByteBuffer *buffer_p, const Address * const address_p, CurlTool *tool_p);

static int AddEscapedValueToByteBuffer (const char *value_s, ByteBuffer *buffer_p, CurlTool *tool_p, const char *prefix_s);



bool RunGoogleGeocoder (Address *address_p, const char *geocoder_uri_s)
{
	bool got_location_flag = false;


	if (address_p -> ad_gps_s)
		{
			/*
			 * Start to try and scan the coords as decimals
			 */
			const char *value_s = address_p -> ad_gps_s;
			double latitude;
			double longitude;
			bool match_flag = false;

			if (GetValidRealNumber (&value_s, &latitude, ","))
				{
					while ((!isdigit (*value_s)) && (*value_s != '-') && (*value_s != '+'))
						{
							++ value_s;
						}

					if (*value_s != '\0')
						{
							if (GetValidRealNumber (&value_s, &longitude, ","))
								{
									while ((isspace (*value_s)) && (*value_s != '\0'))
										{
											++ value_s;
										}

									match_flag = (*value_s == '\0');
								}
						}
				}

			if (match_flag)
				{
					got_location_flag = SetAddressCentreCoordinate (address_p, latitude, longitude, NULL);
				}

		}		/* if (address_p -> ad_gps_s) */
	else
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
						{
							CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

							if (curl_p)
								{
									if (BuildGoogleURLUsingAddressParameter (buffer_p, curl_p, address_p))
										{
											const char *url_s = GetByteBufferData (buffer_p);
											int res = CallGeocoderWebService (curl_p, url_s, address_p, ParseGoogleResults);

											if (res == 1)
												{
													got_location_flag = true;
												}
											else if (res == 0)
												{
													ResetByteBuffer (buffer_p);

													if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
														{
															if (BuildGoogleURLUsingComponentsParameters (buffer_p, address_p, curl_p))
																{
																	url_s = GetByteBufferData (buffer_p);
																	res = CallGeocoderWebService (curl_p, url_s, address_p, ParseGoogleResults);

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

		}		/* if (address_p -> ad_gps_s) else ... */


	return got_location_flag;
}



bool RunGoogleReverseGeocoder (Address *address_p, const char *geocoder_uri_s)
{
	bool success_flag = false;

	if (address_p -> ad_gps_centre_p)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					CurlTool *curl_tool_p = AllocateCurlTool (CM_MEMORY);

					if (curl_tool_p)
						{
							if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
								{
									const char *url_s = GetByteBufferData (buffer_p);
									int res = CallGeocoderWebService (curl_tool_p, url_s, address_p, ParseGoogleResults);

									if (res == 1)
										{
											success_flag = true;
										}
									else if (res == 0)
										{
											ResetByteBuffer (buffer_p);

											if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
												{
													if (BuildGoogleURLUsingComponentsParameters (buffer_p, address_p, curl_tool_p))
														{
															url_s = GetByteBufferData (buffer_p);
															res = CallGeocoderWebService (curl_tool_p, url_s, address_p, ParseGoogleResults);

															if (res == 1)
																{
																	success_flag = true;
																}
														}
												}

										}
								}		/* if (AppendStringToByteBuffer (buffer_p, data_p -> msd_geocoding_uri_s)) */
							else
								{
								}

							FreeCurlTool (curl_tool_p);
						}		/* if (curl_tool_p) */

					FreeByteBuffer (buffer_p);
				}		/* if (buffer_p) */

		}		/* if (address_p -> ad_gps_centre_p) */

	return success_flag;
}



static int ParseGoogleResults (Address *address_p, const json_t *web_service_results_p)
{
	int res = -1;

	/*
	 * Get the status of the response
	 *
	 * 		"OK" indicates that no errors occurred; the address was successfully parsed and at least one geocode was returned.
	 *		"ZERO_RESULTS" indicates that the geocode was successful but returned no results. This may occur if the geocoder was passed a non-existent address.
	 *		"OVER_QUERY_LIMIT" indicates that you are over your quota.
	 *		"REQUEST_DENIED" indicates that your request was denied.
	 *		"INVALID_REQUEST" generally indicates that the query (address, components or latlng) is missing.
	 * 		"UNKNOWN_ERROR" indicates that the request could not be processed due to a server error. The request may succeed if you try again.
	 */
	const char *status_s = GetJSONString (web_service_results_p, "status");

	if (status_s)
		{
			if (strcmp (status_s, "OK") == 0)
				{
					if (RefineLocationDataForGoogle (address_p, web_service_results_p))
						{
							res = 1;
						}
					else
						{
							res = 0;
						}

				}		/* if (strcmp (status_s, "OK") == 0) */
			else if (strcmp (status_s, "ZERO_RESULTS") == 0)
				{
					res = 0;
				}		/* else if (strcmp (status_s, "ZERO_RESULTS") == 0) */
			else if (strcmp (status_s, "OVER_QUERY_LIMIT") == 0)
				{
					res = -1;
				}		/* else if (strcmp (status_s, "OVER_QUERY_LIMIT") == 0) */
			else if (strcmp (status_s, "REQUEST_DENIED") == 0)
				{
					res = -1;
				}		/* else if (strcmp (status_s, "REQUEST_DENIED") == 0) */
			else if (strcmp (status_s, "INVALID_REQUEST") == 0)
				{
					res = -1;
				}		/* else if (strcmp (status_s, "INVALID_REQUEST") == 0) */
			else if (strcmp (status_s, "UNKNOWN_ERROR") == 0)
				{
					res = -1;
				}		/* else if (strcmp (status_s, "UNKNOWN_ERROR") == 0) */
			else
				{
					res = -1;
				}

		}		/* if (status_s) */

	return res;
}




static bool RefineLocationDataForGoogle (Address *address_p, const json_t *raw_google_data_p)
{
	bool success_flag = false;
	const json_t *results_p = json_object_get (raw_google_data_p, "results");

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					const size_t size = json_array_size (results_p);
					size_t i = 0;

					while ((i < size) && (!success_flag))
						{
							const json_t *result_p = json_array_get (results_p, i);

							if (FillInAddressFromGoogleData (address_p, result_p))
								{
									success_flag = true;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "FillInPathogenomicsFromGoogleData failed");
								}

							++ i;
						}		/* while (i < size) */

				}		/* if (json_is_array (results_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "results is not an array");
				}

		}		/* if (results_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, raw_google_data_p, "Failed to get results array");
		}

	return success_flag;
}





bool FillInAddressFromGoogleData (Address *address_p, const json_t *google_result_p)
{
	bool success_flag = false;

	const json_t *geometry_p = json_object_get (google_result_p, "geometry");

	if (geometry_p)
		{
			const json_t *location_p = json_object_get (geometry_p, "location");
			const char * const LATITUDE_KEY_S = "lat";
			const char * const LONGITUDE_KEY_S = "lng";

			#if SAMPLE_METADATA_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, geometry_p, "geometry_p: ");
			#endif

			if (location_p)
				{
					double latitude;

					if (GetJSONReal (location_p, LATITUDE_KEY_S, &latitude))
						{
							double longitude;

							if (GetJSONReal (location_p, LONGITUDE_KEY_S, &longitude))
								{
									if (SetAddressCentreCoordinate (address_p, latitude, longitude, NULL))
										{
											const json_t *viewport_p = json_object_get (geometry_p, "viewport");

											success_flag = true;

											if (viewport_p)
												{
													const json_t *corner_p = json_object_get (viewport_p, "northeast");

													if (corner_p)
														{
															if (GetJSONReal (corner_p, LATITUDE_KEY_S, &latitude))
																{
																	if (GetJSONReal (corner_p, LONGITUDE_KEY_S, &longitude))
																		{
																			if (!SetAddressNorthEastCoordinate (address_p, latitude, longitude, NULL))
																				{
																					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set north east location from %lf %lf ", latitude, longitude);
																				}
																		}
																}
														}

													corner_p = json_object_get (viewport_p, "southwest");

													if (corner_p)
														{
															if (GetJSONReal (corner_p, LATITUDE_KEY_S, &latitude))
																{
																	if (GetJSONReal (corner_p, LONGITUDE_KEY_S, &longitude))
																		{
																			if (! (success_flag = SetAddressSouthWestCoordinate (address_p, latitude, longitude, NULL)))
																				{
																					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set south west location from %lf %lf ", latitude, longitude);
																				}
																		}
																}
														}
												}		/* if (viewport_p) */

										}		/* if (SetAddressCentreCoordinate (address_p, latitude, longitude)) */
								}
						}

				}		/* if (location_p) */

		}		/* if (geometry_p) */


	return success_flag;
}



static bool BuildGoogleURLUsingAddressParameter (ByteBuffer *buffer_p, CurlTool *curl_p, const Address * const address_p)
{
	bool success_flag = false;
	const char *prefix_s = "&address=";
	int res;

	/* name */
	if ((res = AddEscapedValueToByteBuffer (address_p -> ad_name_s, buffer_p, curl_p, prefix_s)) >= 0)
		{
			if (res == 1)
				{
					prefix_s = ",%20";
				}

			/* street */
			if ((res = AddEscapedValueToByteBuffer (address_p -> ad_street_s, buffer_p, curl_p, prefix_s)) >= 0)
				{
					if (res == 1)
						{
							prefix_s = ",%20";
						}

					/* town */
					if ((res = AddEscapedValueToByteBuffer (address_p -> ad_town_s, buffer_p, curl_p, prefix_s)) >= 0)
						{
							if (res == 1)
								{
									prefix_s = ",%20";
								}

							/* county */
							if ((res = AddEscapedValueToByteBuffer (address_p -> ad_county_s, buffer_p, curl_p, prefix_s)) >= 0)
								{
									const char *value_s = address_p -> ad_country_s;

									if (res == 1)
										{
											prefix_s = ",%20";
										}

									if (!value_s)
										{
											value_s = GetCountryNameFromCode (address_p -> ad_country_code_s);

											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get country name for code \"%s\"", address_p -> ad_country_code_s);
										}


									if (value_s)
										{
											if (AppendStringsToByteBuffer (buffer_p, prefix_s, value_s, NULL))
												{
													success_flag = true;
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add country name \"%s\" to buffer for REST API Address call", value_s);
												}
										}

								}		/* if ((res = AddEscapedValueToByteBuffer (address_p -> ad_county_s, buffer_p, tool_p, prefix_s)) >= 0) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add county \"%s\" to buffer for REST API Address call", address_p -> ad_county_s ? address_p -> ad_county_s : "NULL");
								}

						}		/* if ((res = AddEscapedValueToByteBuffer (address_p -> ad_town_s, buffer_p, tool_p, prefix_s)) >= 0 */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add town \"%s\" to buffer for REST API Address call", address_p -> ad_town_s ? address_p -> ad_town_s : "NULL");
						}


				}		/* if ((res = AddEscapedValueToByteBuffer (address_p -> ad_street_s, buffer_p, tool_p, prefix_s)) >= 0) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add street \"%s\" to buffer for REST API Address call", address_p -> ad_street_s ? address_p -> ad_street_s : "NULL");
				}

		}		/* if ((res = AddEscapedValueToByteBuffer (address_p -> ad_name_s, buffer_p, tool_p, prefix_s)) >= 0) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add name \"%s\" to buffer for REST API Address call", address_p -> ad_name_s ? address_p -> ad_name_s : "NULL");
		}


	return success_flag;
}


static bool BuildGoogleURLUsingComponentsParameters (ByteBuffer *buffer_p, const Address * const address_p, CurlTool *tool_p)
{
	bool success_flag = false;
	const char *value_s = address_p -> ad_postcode_s;
	char *escaped_value_s = NULL;


	/* remove any leading spaces */
	while ( (*value_s))
		{
			++ value_s;
		}

	escaped_value_s = GetURLEscapedString (tool_p, value_s);

	if (escaped_value_s)
		{
			success_flag = AppendStringsToByteBuffer (buffer_p, "&components=postal_code:", escaped_value_s, NULL);

			FreeURLEscapedString (escaped_value_s);
		}

	/* country */
	if (success_flag)
		{
			if (address_p -> ad_country_code_s)
				{
					if (IsValidCountryCode (address_p -> ad_country_code_s))
						{
							value_s = address_p -> ad_country_code_s;
						}
				}

			if (!value_s)
				{
					value_s = GetCountryCodeFromName (address_p -> ad_country_s);
				}

			if (value_s)
				{
					success_flag = AppendStringsToByteBuffer (buffer_p, "|country:", value_s, NULL);
				}

		}		/* if (success_flag) */

	return success_flag;
}




static int AddEscapedValueToByteBuffer (const char *value_s, ByteBuffer *buffer_p, CurlTool *tool_p, const char *prefix_s)
{
	int res = -1;

	if (value_s)
		{
			char *escaped_value_s = GetURLEscapedString (tool_p, value_s);

			if (escaped_value_s)
				{
					if (prefix_s)
						{
							res = (AppendStringsToByteBuffer (buffer_p, prefix_s, escaped_value_s, NULL)) ? 1 : -1;
						}		/* if (prefix_s) */
					else
						{
							res = (AppendStringToByteBuffer (buffer_p, escaped_value_s)) ? 1 : -1;
						}

					FreeURLEscapedString (escaped_value_s);
				}
		}		/* if (value_s) */
	else
		{
			res = 0;
		}

	return res;
}
