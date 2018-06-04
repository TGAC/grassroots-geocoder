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
#include "grassroots_config.h"


static GeocoderTool *AllocateGeocoderTool (void);

static void FreeGeocoderTool (GeocoderTool *config_p);

static GeocoderTool *GetGecoderToolFromGrassrootsConfig (void);

static bool RunGeocoderTool (GeocoderTool *tool_p, Address *address_p);



static GeocoderTool *GetGecoderToolFromGrassrootsConfig (void)
{
	GeocoderTool *tool_p = AllocateGeocoderTool ();

	if (tool_p)
		{
			const json_t *geocoder_config_json_p = GetGlobalConfigValue ("geocoder");


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
																tool_p -> gt_geocoder_uri_s = GetJSONString (geocoder_p, "uri");
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
														tool_p -> gt_geocoder_uri_s = GetJSONString (geocoders_p, "uri");
													}
											}

										if (tool_p -> gt_geocoder_uri_s)
											{
												if (strcmp (value_s, "google") == 0)
													{
														tool_p -> gt_callback_fn = DetermineGPSLocationForAddressByGoogle;
													}
												else if (strcmp (value_s, "opencage") == 0)
													{
														tool_p -> gt_callback_fn = DetermineGPSLocationForAddressByOpencage;
													}

												if (tool_p -> gt_callback_fn)
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




bool DetermineGPSLocationForAddress (Address *address_p, GeocoderTool *tool_p)
{
	bool success_flag = false;

	if (!tool_p)
		{
			tool_p = GetGecoderToolFromGrassrootsConfig ();
		}

	if (tool_p)
		{
			success_flag = RunGeocoderTool (tool_p, address_p);
		}		/* if (config_p) */

	return success_flag;
}


bool DetermineGPSLocationForAddressByGoogle (Address *address_p, const char *geocoder_uri_s)
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
					got_location_flag = SetAddressCentreCoordinate (address_p, latitude, longitude);
				}

		}		/* if (address_p -> ad_gps_s) */
	else
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
						{
							bool success_flag = true;
							bool added_query_flag = false;
							const char *country_s = address_p -> ad_country_s;
							const char *value_s;

							/* Replace UK entries with GB */
							if (country_s)
								{
									if (strcmp (country_s, "UK") == 0)
										{
											country_s = "GB";
										}
								}

							/* post code */
							if (address_p -> ad_postcode_s)
								{
									value_s = address_p -> ad_postcode_s;

									/* remove and leading spaces */
									while (isspace (*value_s))
										{
											++ value_s;
										}

									success_flag = (*value_s != '\0') && AppendStringsToByteBuffer (buffer_p, "&components=postal_code:", value_s, NULL);

									/* country */
									if (success_flag)
										{
											if (country_s)
												{
													if (IsValidCountryCode (country_s))
														{
															value_s = country_s;
														}
													else
														{
															value_s = GetCountryCodeFromName (country_s);
														}

													if (value_s)
														{
															success_flag = AppendStringsToByteBuffer (buffer_p, "|country:", value_s, NULL);
														}


												}

										}		/* if (success_flag) */

								}		/* if (postcode_s) */
							else
								{
									/* town */
									if (address_p -> ad_town_s)
										{
											success_flag = AppendStringsToByteBuffer (buffer_p, "&address=", address_p -> ad_town_s, NULL);
											added_query_flag = true;
										}

									/* county */
									if (success_flag)
										{
											if (address_p -> ad_county_s)
												{
													if (added_query_flag)
														{
															success_flag = AppendStringsToByteBuffer (buffer_p, ",%20", address_p -> ad_county_s, NULL);
														}
													else
														{
															success_flag = AppendStringsToByteBuffer (buffer_p, "&address=", address_p -> ad_county_s, NULL);
															added_query_flag = true;
														}
												}		/* if (county_s) */

										}		/* if (success_flag) */


									/* country */
									if (success_flag)
										{
											if (country_s)
												{
													if (IsValidCountryCode (country_s))
														{
															value_s = country_s;
														}
													else
														{
															value_s = GetCountryCodeFromName (country_s);
														}

													if (value_s)
														{
															success_flag = AppendStringsToByteBuffer (buffer_p, "&components=country:", value_s, NULL);
														}
												}
										}

								}

							if (success_flag)
								{
									CurlTool *curl_tool_p = AllocateCurlTool (CM_MEMORY);

									if (curl_tool_p)
										{
											const char *uri_s = NULL;

											ReplaceCharsInByteBuffer (buffer_p, ' ', '+');
											uri_s = GetByteBufferData (buffer_p);

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
																			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, raw_res_p, "raw: ");
																			got_location_flag = RefineLocationDataForGoogle (address_p, raw_res_p);

																			json_decref (raw_res_p);
																		}
																	else
																		{

																		}
																}
														}
													else
														{

														}
												}


											FreeCurlTool (curl_tool_p);
										}		/* if (curl_tool_p) */

								}

						}		/* if (AppendStringToByteBuffer (buffer_p, data_p -> msd_geocoding_uri_s)) */

					FreeByteBuffer (buffer_p);
				}		/* if (buffer_p) */

		}		/* if (address_p -> ad_gps_s) else ... */


	return got_location_flag;
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
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					if (AppendStringToByteBuffer (buffer_p, geocoder_uri_s))
						{
							bool success_flag = true;
							bool added_query_flag = false;

							/* town */
							if (address_p -> ad_town_s)
								{
									success_flag = AppendStringsToByteBuffer (buffer_p, "&query=", address_p -> ad_town_s, NULL);
									added_query_flag = true;
								}

							/* county */
							if (success_flag)
								{
									if (address_p -> ad_county_s)
										{
											if (added_query_flag)
												{
													success_flag = AppendStringsToByteBuffer (buffer_p, ",%20", address_p -> ad_county_s, NULL);
												}
											else
												{
													success_flag = AppendStringsToByteBuffer (buffer_p, "&query=", address_p -> ad_county_s, NULL);
													added_query_flag = true;
												}
										}		/* if (county_s) */

								}		/* if (success_flag) */


							/* country */
							if (success_flag)
								{
									const char *country_code_s = NULL;

									if (address_p -> ad_country_code_s)
										{
											if (IsValidCountryCode (address_p -> ad_country_code_s))
												{
													country_code_s = address_p -> ad_country_code_s;
												}
											else
												{
													if (address_p -> ad_country_s)
														{
															country_code_s = GetCountryCodeFromName (address_p -> ad_country_s);
														}
												}

											if (country_code_s)
												{
													success_flag = AppendStringsToByteBuffer (buffer_p, "&countrycode=", country_code_s, NULL);
												}

										}

								}		/* if (success_flag) */


							if (success_flag)
								{
									CurlTool *curl_tool_p = AllocateCurlTool (CM_MEMORY);

									if (curl_tool_p)
										{
											const char *uri_s = GetByteBufferData (buffer_p);

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
																			char *dump_s = json_dumps (res_p, JSON_INDENT (2) | JSON_PRESERVE_ORDER);

																			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "json:\n%s\n", dump_s);
																			free (dump_s);

																			/*
																			res_p = data_p -> msd_refine_location_fn (data_p, raw_res_p, town_s, county_s, country_code_s);

																			WipeJSON (raw_res_p);
																			 */

																			/** TODO */
																			res_p = raw_res_p;
																		}
																	else
																		{

																		}
																}
														}
													else
														{

														}
												}


											FreeCurlTool (curl_tool_p);
										}		/* if (curl_tool_p) */

								}

						}		/* if (AppendStringToByteBuffer (buffer_p, data_p -> msd_geocoding_uri_s)) */

					FreeByteBuffer (buffer_p);
				}		/* if (buffer_p) */

		}

	return got_location_flag;
}



bool RefineLocationDataForGoogle (Address *address_p, const json_t *raw_google_data_p)
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
									if (SetAddressCentreCoordinate (address_p, latitude, longitude))
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
																			if (!SetAddressNorthEastCoordinate (address_p, latitude, longitude))
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
																			if (! (success_flag = SetAddressSouthWestCoordinate (address_p, latitude, longitude)))
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



static bool RunGeocoderTool (GeocoderTool *tool_p, Address *address_p)
{
	bool success_flag = false;

	if ((tool_p -> gt_callback_fn) && (tool_p -> gt_geocoder_uri_s))
		{
			success_flag = tool_p -> gt_callback_fn (address_p, tool_p -> gt_geocoder_uri_s);
		}

	return success_flag;
}


static GeocoderTool *AllocateGeocoderTool (void)
{
	GeocoderTool *config_p = (GeocoderTool *) AllocMemory (sizeof (GeocoderTool));

	if (config_p)
		{
			config_p -> gt_callback_fn = NULL;
			config_p -> gt_geocoder_uri_s = NULL;
		}

	return config_p;
}


static void FreeGeocoderTool (GeocoderTool *config_p)
{
	FreeMemory (config_p);
}

