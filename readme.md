# Geocoder library {#geocoder_library}

The Geocoder library is for getting GPS coordinates from address details such as town, county, country, etc. and vice versa.

## Installation

To build this library, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. 

The files to build the Geocoder library are in the ```build/<platform>``` directory. 

### Linux

Enter the build directory 

```
cd build/linux
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


## Configuration options



~~~{json}
{
 "geocoder": {
	"default_geocoder": "google",
	"geocoders": [{
		"name": "google",
		"uri": "https://maps.googleapis.com/maps/api/geocode/json?sensor=false&key=<api key>"
	}, {
		"name": "opencage",
		"uri": "http://api.opencagedata.com/geocode/v1/json?pretty=1&key=<api_key>"
	}],
 }
...

}
~~~
