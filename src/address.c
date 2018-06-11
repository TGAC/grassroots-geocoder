/*
 * address.c
 *
 *  Created on: 31 May 2018
 *      Author: billy
 */

#include <ctype.h>
#include <string.h>

#define ALLOCATE_GEOCODER_TAGS (1)
#include "address.h"

#include "memory_allocations.h"
#include "string_utils.h"
#include "streams.h"
#include "math_utils.h"
#include "byte_buffer.h"
#include "curl_tools.h"
#include "country_codes.h"
#include "json_util.h"


static bool AddValidJSONField (json_t *json_p, const char *key_s, const char *value_s);

static bool SetCoordinateValue (Coordinate **coord_pp, const double64 latitude, const double64 longitude);




Address *AllocateAddress (const char *name_s, const char *street_s, const char *town_s, const char *county_s, const char *country_s, const char *postcode_s, const char *country_code_s, const char *gps_s)
{
	Address *address_p = NULL;

	char *copied_name_s = NULL;

	if (CloneValidString (name_s, &copied_name_s))
		{
			char *copied_street_s = NULL;

			if (CloneValidString (street_s, &copied_street_s))
				{
					char *copied_town_s = NULL;

					if (CloneValidString (town_s, &copied_town_s))
						{
							char *copied_county_s = NULL;

							if (CloneValidString (county_s, &copied_county_s))
								{
									char *copied_country_s = NULL;

									if (CloneValidString (country_s, &copied_country_s))
										{
											char *copied_postcode_s = NULL;

											if (CloneValidString (postcode_s, &copied_postcode_s))
												{
													char *copied_country_code_s = NULL;

													if (CloneValidString (country_code_s, &copied_country_code_s))
														{
															char *copied_gps_s = NULL;

															if (CloneValidString (gps_s, &copied_gps_s))
																{
																	address_p = (Address *) AllocMemory (sizeof (Address));

																	if (address_p)
																		{
																			address_p -> ad_name_s = copied_name_s;
																			address_p -> ad_street_s = copied_street_s;
																			address_p -> ad_town_s = copied_town_s;
																			address_p -> ad_county_s = copied_county_s;
																			address_p -> ad_country_s = copied_country_s;
																			address_p -> ad_postcode_s = copied_postcode_s;
																			address_p -> ad_country_code_s = copied_country_code_s;
																			address_p -> ad_gps_s = copied_gps_s;
																			address_p -> ad_gps_centre_p = NULL;
																			address_p -> ad_gps_north_east_p = NULL;
																			address_p -> ad_gps_south_west_p = NULL;

																			return address_p;
																		}

																	FreeCopiedString (copied_gps_s);
																}		/* if (CloneValidString (gps_s, &copied_gps_s)) */

															FreeCopiedString (copied_country_code_s);
														}		/* if (CloneValidString (country_code_s, &copied_country_code_s)) */


													FreeCopiedString (copied_postcode_s);
												}		/* if (CloneValidString (postcode_s, &copied_postcode_s)) */


											FreeCopiedString (copied_country_s);
										}		/* if (CloneValidString (country_s, &copied_country_s)) */


									FreeCopiedString (copied_county_s);
								}		/* if (CloneValidString (county_s, &copied_county_s)) */


							FreeCopiedString (copied_town_s);
						}		/* if (CloneValidString (town_s, &copied_town_s)) */

					FreeCopiedString (copied_street_s);
				}		/* if (CloneValidString (street_s, &copied_street_s)) */

		}		/* if (CloneValidString (name_s, &copied_name_s */

	return NULL;
}


void FreeAddress (Address *address_p)
{
	ClearAddress (address_p);
	FreeMemory (address_p);
}


void ClearAddress (Address *address_p)
{
	FreeCopiedString (address_p -> ad_country_code_s);
	FreeCopiedString (address_p -> ad_country_s);
	FreeCopiedString (address_p -> ad_county_s);
	FreeCopiedString (address_p -> ad_gps_s);
	FreeCopiedString (address_p -> ad_postcode_s);
	FreeCopiedString (address_p -> ad_town_s);
	FreeCopiedString (address_p -> ad_street_s);
	FreeCopiedString (address_p -> ad_name_s);

	if (address_p -> ad_gps_centre_p)
		{
			FreeCoordinate (address_p -> ad_gps_centre_p);
		}

	if (address_p -> ad_gps_north_east_p)
		{
			FreeCoordinate (address_p -> ad_gps_north_east_p);
		}

	if (address_p -> ad_gps_south_west_p)
		{
			FreeCoordinate (address_p -> ad_gps_south_west_p);
		}

	memset (address_p, 0, sizeof (Address));
}


json_t *GetAddressAsJSON (const Address *address_p)
{
	json_t *address_json_p = json_object ();

	if (address_json_p)
		{
			if (ConvertAddressToJSON (address_p, address_json_p))
				{
					return address_json_p;
				}

			json_decref (address_json_p);
		}

	return NULL;
}


bool ConvertAddressToJSON (const Address *address_p, json_t *dest_p)
{
	json_t *location_p = json_object ();

	if (location_p)
		{
			if (AddCoordinateToJSON (address_p -> ad_gps_centre_p, location_p, AD_LOCATION_S))
				{
					if (AddCoordinateToJSON (address_p -> ad_gps_north_east_p, location_p, AD_NORTH_EAST_LOCATION_S))
						{
							if (AddCoordinateToJSON (address_p -> ad_gps_south_west_p, location_p, AD_SOUTH_WEST_LOCATION_S))
								{
									if (ParseAddressForSchemaOrg (address_p, dest_p, AD_ADDRESS_S))
										{
											if (json_object_set_new (dest_p, AD_LOCATION_S, location_p) == 0)
												{
													return true;
												}		/* if (json_object_set_new (dest_p, AD_LOCATION_S, location_p) == 0) */

										}		/* if (ParseAddressForSchemaOrg (address_p, dest_p, AD_ADDRESS_S)) */

								}		/* if (AddCoordinateToJSON (address_p -> ad_gps_centre_p, location_p, AD_LOCATION_S)) */

						}		/* if (AddCoordinateToJSON (address_p -> ad_gps_centre_p, location_p, AD_LOCATION_S)) */

				}		/* if (AddCoordinateToJSON (address_p -> ad_gps_centre_p, location_p, AD_LOCATION_S)) */

			json_decref (location_p);
		}		/* if (location_p) */


	return false;
}




bool ParseAddressForSchemaOrg (const Address *address_p, json_t *values_p, const char *address_key_s)
{
	bool success_flag = false;

	if (address_p -> ad_town_s || address_p -> ad_county_s || address_p -> ad_country_s || address_p -> ad_postcode_s)
		{
			json_t *postal_address_p = json_object ();

			success_flag = false;

			if (postal_address_p)
				{
					if (json_object_set_new (postal_address_p, "@type", json_string ("PostalAddress")) == 0)
						{
							if (AddValidJSONField (postal_address_p, "name", address_p -> ad_name_s))
								{
									success_flag = true;
								}

							if (AddValidJSONField (postal_address_p, "streetAddress", address_p -> ad_street_s))
								{
									success_flag = true;
								}

							if (AddValidJSONField (postal_address_p, "addressLocality", address_p -> ad_town_s))
								{
									success_flag = true;
								}

							if (AddValidJSONField (postal_address_p, "addressRegion", address_p -> ad_county_s))
								{
									success_flag = true;
								}

							if (AddValidJSONField (postal_address_p, "addressCountry", address_p -> ad_country_s))
								{
									success_flag = true;
								}

							if (AddValidJSONField (postal_address_p, "postalCode", address_p -> ad_postcode_s))
								{
									success_flag = true;
								}

							if (success_flag)
								{
									success_flag = (json_object_set_new (values_p, address_key_s, postal_address_p) == 0);
								}
						}

					if (!success_flag)
						{
							json_decref (postal_address_p);
						}
				}

		}		/* if (town_s || county_s || country_s || postcode_s) */


	return success_flag;
}


bool SetAddressCentreCoordinate (Address *address_p, const double64 latitude, const double64 longitude)
{
	return SetCoordinateValue (& (address_p -> ad_gps_centre_p), latitude, longitude);
}


bool SetAddressNorthEastCoordinate (Address *address_p, const double64 latitude, const double64 longitude)
{
	return SetCoordinateValue (& (address_p -> ad_gps_north_east_p), latitude, longitude);
}


bool SetAddressSouthWestCoordinate (Address *address_p, const double64 latitude, const double64 longitude)
{
	return SetCoordinateValue (& (address_p -> ad_gps_south_west_p), latitude, longitude);
}




static bool SetCoordinateValue (Coordinate **coord_pp, const double64 latitude, const double64 longitude)
{

	bool success_flag = true;

	if (*coord_pp)
		{
			(*coord_pp) -> po_x = latitude;
			(*coord_pp) -> po_y = longitude;
		}
	else
		{
			*coord_pp = AllocateCoordinate (latitude, longitude);

			if (! (*coord_pp))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate cooordinate");
					success_flag = false;
				}
		}

	return success_flag;
}


static bool AddValidJSONField (json_t *json_p, const char *key_s, const char *value_s)
{
	bool success_flag = true;

	if (key_s && value_s)
		{
			success_flag = (json_object_set_new (json_p, key_s, json_string (value_s)) == 0);
		}

	return success_flag;
}


