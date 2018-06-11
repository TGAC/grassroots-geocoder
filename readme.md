# Geocoder library {#geocoder_library}

The Geocoder library is for getting GPS coordinates from address details such as town, county, country, *etc.* and vice versa.

## Installation

To build this library, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) modules.

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


The configuration options for this library are specified in the global configuration file ```grassroots.config``` in the value associated with the ```geocoder``` key. It has an array of geocoder configuration details specified by the ```geocoders``` key. Each one of these consists of two entries:

 * ***name***: The name to use for this geocoder. Currently there are two available options; one supplied by Google and the other by Opencage.
 * ***uri***: This is the web address to call to get the geocoding details. You will need to get an API key from the appropriate vendor and assign its value to the key parameter in this address. 


The other key is ```default_geocoder``` and the associated value needs to be one of the names of the entries in the ```geocoders``` array. 

An example configuration section is shown below with ```<api key>``` being where your appropriate vendor key should go.

~~~{json}
{
  "geocoder": {
	"default_geocoder": "google",
	"geocoders": [{
		"name": "google",
		"uri": "https://maps.googleapis.com/maps/api/geocode/json?sensor=false&key=<api key>"
	}, {
		"name": "opencage",
		"uri": "http://api.opencagedata.com/geocode/v1/json?pretty=1&key=<api key>"
	}],
  }
...

}
~~~
