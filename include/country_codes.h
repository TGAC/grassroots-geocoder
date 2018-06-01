/*
** Copyright 2014-2016 The Earlham Institute
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

/**
 * @file
 * @brief
 */
#ifndef COUNTRY_CODES_H
#define COUNTRY_CODES_H

#include "grassroots_geocoder_library.h"

#include "typedefs.h"

typedef struct CountryCode CountryCode;

/**
 * @private
 *
 */
struct CountryCode
{
	const char *cc_name_s;
	const char *cc_code_s;
};


#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_GEOCODER_API const char *GetCountryCodeFromName (const char * const country_name_s);


GRASSROOTS_GEOCODER_API bool IsValidCountryCode (const char * const code_s);


//GRASSROOTS_UTIL_API bool GetLocationData (MongoTool *tool_p, json_t *row_p, PathogenomicsServiceData *data_p, const char *id_s);


//PATHOGENOMICS_SERVICE_LOCAL const char * InsertLocationData (MongoTool *tool_p, const json_t *row_p, PathogenomicsServiceData *data_p, const char *id_s);


#ifdef __cplusplus
}
#endif

#endif 		/* ifnder COUNTRY_CODES_H */
 
