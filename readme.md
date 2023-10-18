# Geocoder library 

The Geocoder library is for getting GPS coordinates from address details such as town, county, country, *etc.* and vice versa.

## Installation

To build this library, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) modules.

The files to build the Geocoder library are in the `build` directory. 

### Linux and Mac

Enter the build directory for your your given platform which is inside the `build/unix/<platform>` 

For example, under linux:

```
cd build/unix/linux
```

whereas on MacOS:

```
cd build/unix/mac
```

then

```
make all
```

and then 

```
make install
```

to install the library into the Grassroots system where it will be available for use immediately.


### Windows

Under Windows, there is a Visual Studio project in the `build/windows` folder that allows you to build the geocoder library.


## Configuration options


The configuration options for this library are specified with the `geocoder` key within teh global Grassroots configuration file. It has an array of geocoder configuration details specified by the geocoders key. Each one of these consists of two entries:

 * **name**: The name to use for this geocoder. Currently there are three available options; [google](https://developers.google.com/maps/documentation/geocoding/overview), [opencage](https://opencagedata.com/api) and [nominatim](https://nominatim.org/).

 * **geocode_url**: This is the web address to call when you have address details and wish to determine the corresponding GPS coordinates. These uri values are vendor-dependent and you will need to get an API key from the appropriate vendor and assign its value to the key parameter in this address.
       
 * **reverse_geocode_url**: This is the web address to call to when you have some GPS coordinates and wish to discover the corresponding address. These uri values are vendor-dependent and you will need to get an API key from the appropriate vendor and assign its value to the key parameter in this address.

The other key is `default_geocoder` which specifies which and the associated value needs to be one of the names of the entries in the `geocoders` array.

An example configuration section is shown below with `<api key>` being where your appropriate vendor key should go.

~~~{json}
{
	"geocoder": {
		"default_geocoder": "nominatim",
		"geocoders": [{
			"name": "nominatim",
			"reverse_geocode_url": "https://nominatim.openstreetmap.org/reverse",
			"geocode_url": "https://nominatim.openstreetmap.org/search?format=json"
		}, {
			"name": "google",
			"geocode_url": "https://maps.googleapis.com/maps/api/geocode/json?sensor=false&key=my_google_key"
		}, {
			"name": "opencage",
			"geocode_url": "https://api.opencagedata.com/geocode/v1/json?key=my_opencage_keyd&pretty=1&q="
		}]
	}
...

}
~~~


So, for example, if your Google Geocoding API Key is *123Google* and your OpenCage key is *456OpenCage*, and you wanted to use the Google Geocoder by default, then the config would be: 


~~~{json}
{
  "geocoder": {
	"default_geocoder": "google",
	"geocoders": [{
		"name": "google",
		"geocode_url": "https://maps.googleapis.com/maps/api/geocode/json?sensor=false&key=123Google"
	}, {
		"name": "opencage",
		"geocode_url": "http://api.opencagedata.com/geocode/v1/json?pretty=1&key=456OpenCage"
	}],
  }
...

}
~~~