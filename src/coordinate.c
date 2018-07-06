/*
 * coordinate.c
 *
 *  Created on: 31 May 2018
 *      Author: billy
 */

#define ALLOCATE_COORDINATE_TAGS (1)
#include "coordinate.h"
#include "memory_allocations.h"
#include "streams.h"
#include "json_util.h"


Coordinate *AllocateCoordinate (double64 x, double64 y)
{
	Coordinate *coord_p = (Coordinate *) AllocMemory (sizeof (Coordinate));

	if (coord_p)
		{
			coord_p -> po_x = x;
			coord_p -> po_y = y;
		}

	return coord_p;
}


void FreeCoordinate (Coordinate *coord_p)
{
	FreeMemory (coord_p);
}


bool AddCoordinateToJSON (const Coordinate *coord_p, json_t *dest_p, const char * const coord_key_s)
{
	json_t *json_p = GetCoordinateAsJSON (coord_p);

	if (json_p)
		{
			if (json_object_set_new (dest_p, coord_key_s, json_p) == 0)
				{
					return true;
				}

			json_decref (json_p);
		}

	return false;
}


bool SetCoordinateFromJSON (Coordinate *coord_p, const json_t *value_p)
{
	bool success_flag = false;
	double latitude;

	if (GetJSONReal (value_p, CO_LATITUDE_S, &latitude))
		{
			double longitude;

			if (GetJSONReal (value_p, CO_LONGITUDE_S, &longitude))
				{
					coord_p -> po_x = latitude;
					coord_p -> po_y = longitude;

					success_flag = true;
				}

		}		/* if (location_p) */


	return success_flag;
}



json_t *GetCoordinateAsJSON (const Coordinate * const coord_p)
{
	json_t *coord_json_p = json_object ();

	if (coord_json_p)
		{
			const char * const TYPE_S = "so:GeoCoordinates";

			if (json_object_set_new (coord_json_p, "@type", json_string (TYPE_S)) == 0)
				{
					if (json_object_set_new (coord_json_p, CO_LATITUDE_S, json_real (coord_p -> po_x)) == 0)
						{
							if (json_object_set_new (coord_json_p, CO_LATITUDE_S, json_real (coord_p -> po_y)) == 0)
								{
									return coord_json_p;
								}		/* if (json_object_set_new (coord_json_p, "longitude", json_real (coord_p -> po_y)) == 0) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add longitude to JSON");
								}

						}		/* if (json_object_set_new (coord_json_p, "latitude", json_real (coord_p -> po_x)) == 0) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add latitude to JSON");
						}

				}		/* if (json_object_set_new (coord_json_p, "@type", json_string ("so:GeoCoordinates")) == 0) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add @type of %s to JSON", TYPE_S);
				}

			json_decref (coord_json_p);
		}		/* if (coord_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create JSON object for Coordinate");
		}

	return NULL;
}
