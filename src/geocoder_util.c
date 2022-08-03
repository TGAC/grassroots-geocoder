/*
 * geocoder_util.c
 *
 *  Created on: 30 May 2018
 *      Author: billy
 */

#include <ctype.h>
#include <string.h>

#include "typedefs.h"
#include "math_utils.h"
#include "streams.h"
#include "memory_allocations.h"
#include "json_util.h"
#include "geocoder_util.h"
#include "byte_buffer.h"
#include "curl_tools.h"
#include "country_codes.h"
#include "address.h"
#include "coordinate.h"
#include "grassroots_server.h"
#include "string_utils.h"

#include "google.h"
#include "nominatim.h"


static GeocoderTool *AllocateGeocoderTool (void);

static void FreeGeocoderTool (GeocoderTool *config_p);

static GeocoderTool *GetGecoderToolFromGrassrootsConfig (GrassrootsServer *grassroots_p);

static bool DoGeocoding (GeocoderTool *tool_p, Address *address_p);

static bool DoReverseGeocoding (GeocoderTool *tool_p, Address *address_p);



static bool SetCoordinateFromOpencage (const json_t *coords_p, Address *address_p, bool (*set_coord_fn) (Address *address_p, const double64 latitude, const double64 longitude, const double64 *elevation_p));



static GeocoderTool *GetGecoderToolFromGrassrootsConfig (GrassrootsServer *grassroots_p)
{
	GeocoderTool *tool_p = AllocateGeocoderTool ();

	if (tool_p)
		{
			const json_t *geocoder_config_json_p = GetGlobalConfigValue (grassroots_p, "geocoder");


				if (geocoder_config_json_p)
					{
						const char *value_s = GetJSONString (geocoder_config_json_p, "default_geocoder");

						if (value_s)
							{
								json_t *geocoders_p = json_object_get (geocoder_config_json_p, "geocoders");

								if (geocoders_p)
									{
										if (json_is_array (geocoders_p))
											{
												const size_t size = json_array_size (geocoders_p);
												size_t i = 0;

												while (i < size)
													{
														json_t *geocoder_p = json_array_get (geocoders_p, i);
														const char *name_s = GetJSONString (geocoder_p, "name");

														if (name_s && (strcmp (name_s, value_s) == 0))
															{
																tool_p -> gt_geocoder_url_s = GetJSONString (geocoder_p, "geocode_url");
																tool_p -> gt_reverse_geocoder_url_s = GetJSONString (geocoder_p, "reverse_geocode_url");
																i = size;
															}
														else
															{
																++ i;
															}
													}
											}
										else
											{
												const char *name_s = GetJSONString (geocoders_p, "name");

												if (name_s && (strcmp (name_s, value_s) == 0))
													{
														tool_p -> gt_geocoder_url_s = GetJSONString (geocoders_p, "geocode_url");
													}
											}

										if (tool_p -> gt_geocoder_url_s)
											{
												if (Stricmp (value_s, "google") == 0)
													{
														tool_p -> gt_geocoder_fn = RunGoogleGeocoder;
													}
												else if (Stricmp (value_s, "opencage") == 0)
													{
														tool_p -> gt_geocoder_fn = DetermineGPSLocationForAddressByOpencage;
													}
												else if (Stricmp (value_s, "locationiq") == 0)
													{
														tool_p -> gt_geocoder_fn = DetermineGPSLocationForAddressByLocationIQ;
													}
												else if (Stricmp (value_s, "nominatim") == 0)
													{
														tool_p -> gt_geocoder_fn = RunNominatimGeocoder;

														if (tool_p -> gt_reverse_geocoder_url_s)
															{
																tool_p -> gt_reverse_geocoder_fn = RunNominatimReverseGeocoder;
															}
													}

												if (tool_p -> gt_geocoder_fn)
													{
														return tool_p;
													}

											}
									}
							}

					}		/* if (geocoder_config_json_p) */

				FreeGeocoderTool (tool_p);

		}		/* if (tool_p) */

	return NULL;
}




bool DetermineGPSLocationForAddress (Address *address_p, GeocoderTool *tool_p, GrassrootsServer *grassroots_p)
{
	bool success_flag = false;

	if (!tool_p)
		{
			tool_p = GetGecoderToolFromGrassrootsConfig (grassroots_p);
		}

	if (tool_p)
		{
			success_flag = DoGeocoding (tool_p, address_p);
		}		/* if (config_p) */

	return success_flag;
}


bool DetermineAddressForGPSLocation (Address *address_p, GeocoderTool *tool_p, GrassrootsServer *grassroots_p)
{
	bool success_flag = false;

	if (!tool_p)
		{
			tool_p = GetGecoderToolFromGrassrootsConfig (grassroots_p);
		}

	if (tool_p)
		{
			success_flag = DoReverseGeocoding (tool_p, address_p);
		}		/* if (config_p) */

	return success_flag;
}


int CallGeocoderWebService (CurlTool *curl_tool_p, const char *url_s, Address *address_p, int (*parse_results_fn) (Address *address_p, const json_t *web_service_results_p))
{
	int res = -1;
	char *escaped_url_s = EasyCopyToNewString (url_s);

	if (escaped_url_s)
		{
			ReplaceChars (escaped_url_s, ' ', '+');

			if (SetUriForCurlTool (curl_tool_p, escaped_url_s))
				{
					CURLcode c = RunCurlTool (curl_tool_p);

					if (c == CURLE_OK)
						{
							const char *response_s = GetCurlToolData (curl_tool_p);

							if (response_s)
								{
									json_error_t error;
									json_t *raw_res_p = NULL;

									PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "geo response for %s\n%s\n", url_s, response_s);

									raw_res_p = json_loads (response_s, 0, &error);

									if (raw_res_p)
										{
											res = parse_results_fn (address_p, raw_res_p);

											json_decref (raw_res_p);
										}		/* if (raw_res_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse \"%s\" as json from \"%s\"", response_s, url_s);
										}

								}		/* if (response_s) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get response data from CurlTool for \"%s\"", url_s);
								}

						}		/* if (c == CURLE_OK) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error calling \"%s\"for CurlTool, error \"%s\"", url_s, curl_easy_strerror (c));
						}

				}		/* if (SetUriForCurlTool (curl_tool_p, url_s)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set URL for CurlTool to \"%s\"", url_s);
				}

		}		/* if (escaped_url_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate CurlTool for \"%s\"", url_s);
		}


	return res;
}


int AddEscapedValueToByteBuffer (const char *value_s, ByteBuffer *buffer_p, CurlTool *tool_p, const char *prefix_s)
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



bool BuildURLUsingAddressParameter (ByteBuffer *buffer_p, CurlTool *curl_p, const Address * const address_p, const char *api_call_s, const char *sep_s)
{
	bool success_flag = false;
	const char *prefix_s = api_call_s;
	int res;

	/* name */
	if ((res = AddEscapedValueToByteBuffer (address_p -> ad_name_s, buffer_p, curl_p, prefix_s)) >= 0)
		{
			if (res == 1)
				{
					prefix_s = sep_s;
				}

			/* street */
			if ((res = AddEscapedValueToByteBuffer (address_p -> ad_street_s, buffer_p, curl_p, prefix_s)) >= 0)
				{
					if (res == 1)
						{
							prefix_s = sep_s;
						}

					/* town */
					if ((res = AddEscapedValueToByteBuffer (address_p -> ad_town_s, buffer_p, curl_p, prefix_s)) >= 0)
						{
							if (res == 1)
								{
									prefix_s = sep_s;
								}

							/* county */
							if ((res = AddEscapedValueToByteBuffer (address_p -> ad_county_s, buffer_p, curl_p, prefix_s)) >= 0)
								{
									const char *value_s = address_p -> ad_country_s;

									if (res == 1)
										{
											prefix_s = sep_s;
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



bool DetermineGPSLocationForAddressByOpencage (Address *address_p, const char *geocoder_uri_s)
{
	bool got_location_flag = false;
	json_t *res_p = NULL;

	if (address_p -> ad_gps_centre_p)
		{

		}
	else
		{
			char *address_s = GetAddressAsDelimitedString (address_p, ",+");

			if (address_s)
				{
					CurlTool *curl_tool_p = AllocateMemoryCurlTool (0);

					if (curl_tool_p)
						{
							char *uri_s = NULL;

							/*
							 * For the url request, spaces are encoded as +
							 */
							ReplaceChars (address_s, ' ', '+');

							if (address_p -> ad_country_code_s)
								{
									uri_s = ConcatenateVarargsStrings (geocoder_uri_s, address_s, "&countrycode=", address_p -> ad_country_code_s, NULL);

								}
							else
								{
									uri_s = ConcatenateStrings (geocoder_uri_s, address_s);
								}

							if (uri_s)
								{
									if (SetUriForCurlTool (curl_tool_p, uri_s))
										{
											CURLcode c = RunCurlTool (curl_tool_p);

											if (c == CURLE_OK)
												{
													const char *response_s = GetCurlToolData (curl_tool_p);

													if (response_s)
														{
															json_error_t error;
															json_t *raw_res_p = NULL;

															PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "geo response for %s\n%s\n", uri_s, response_s);

															raw_res_p = json_loads (response_s, 0, &error);

															if (raw_res_p)
																{
																	const json_t *results_p = json_object_get (raw_res_p, "results");

																	if (results_p)
																		{
																			if (json_is_array (results_p))
																				{
																					size_t i = 0;
																					const size_t size = json_array_size (results_p);
																					bool done_flag = false;

																					while (i < size)
																						{
																							const json_t *result_p = json_array_get (results_p, i);

																							/*
																							 * Get the centre
																							 */
																							if (SetCoordinateFromOpencage (json_object_get (result_p, "geometry"), address_p, SetAddressCentreCoordinate))
																								{
																									const json_t *bounds_p = json_object_get (result_p, "bounds");

																									got_location_flag = true;

																									if (bounds_p)
																										{
																											if (SetCoordinateFromOpencage (json_object_get (result_p, "northeast"), address_p, SetAddressNorthEastCoordinate))
																												{
																													if (SetCoordinateFromOpencage (json_object_get (result_p, "southwest"), address_p, SetAddressNorthEastCoordinate))
																														{
																															done_flag = true;
																														}

																												}
																										}
																								}

																							if (done_flag)
																								{
																									i = size; 		/* force exit from loop */
																								}
																							else
																								{
																									++ i;
																								}

																						}		/* while (i < size) */

																				}

																		}		/* if (results_p) */

																	json_decref (raw_res_p);
																}		/* if (raw_res_p) */
															else
																{

																}

														}		/* if (response_s) */

												}		/* if (c == CURLE_OK) */
											else
												{

												}

										}		/* if (SetUriForCurlTool (curl_tool_p, uri_s)) */

									FreeCopiedString (uri_s);
								}		/* if (uri_s) */


							FreeCurlTool (curl_tool_p);
						}		/* if (curl_tool_p) */

					FreeCopiedString (address_s);
				}		/* if (address_s) */

		}

	return got_location_flag;
}



static bool SetCoordinateFromOpencage (const json_t *coords_p, Address *address_p, bool (*set_coord_fn) (Address *address_p, const double64 latitude, const double64 longitude, const double64 *elevation_p))
{
	bool success_flag = false;

	if (coords_p)
		{
			double latitude;

			if (GetJSONReal (coords_p, "lat", &latitude))
				{
					double longitude;

					if (GetJSONReal (coords_p, "lng", &longitude))
						{
							if (set_coord_fn (address_p, latitude, longitude, NULL))
								{
									success_flag = true;
								}
						}
				}

		}		/* if (coords_p) */

	return success_flag;
}


bool DetermineGPSLocationForAddressByLocationIQ (Address *address_p, const char *geocoder_uri_s)
{
	bool success_flag = false;

	return success_flag;
}




static bool DoGeocoding (GeocoderTool *tool_p, Address *address_p)
{
	bool success_flag = false;

	if ((tool_p -> gt_geocoder_fn) && (tool_p -> gt_geocoder_url_s))
		{
			success_flag = tool_p -> gt_geocoder_fn (address_p, tool_p -> gt_geocoder_url_s);
		}

	return success_flag;
}


static bool DoReverseGeocoding (GeocoderTool *tool_p, Address *address_p)
{
	bool success_flag = false;

	if ((tool_p -> gt_reverse_geocoder_fn) && (tool_p -> gt_reverse_geocoder_url_s))
		{
			success_flag = tool_p -> gt_reverse_geocoder_fn (address_p, tool_p -> gt_reverse_geocoder_url_s);
		}

	return success_flag;
}



static GeocoderTool *AllocateGeocoderTool (void)
{
	GeocoderTool *config_p = (GeocoderTool *) AllocMemory (sizeof (GeocoderTool));

	if (config_p)
		{
			config_p -> gt_geocoder_fn = NULL;
			config_p -> gt_reverse_geocoder_fn = NULL;
			config_p -> gt_geocoder_url_s = NULL;
			config_p -> gt_reverse_geocoder_url_s = NULL;
		}

	return config_p;
}


static void FreeGeocoderTool (GeocoderTool *config_p)
{
	FreeMemory (config_p);
}

