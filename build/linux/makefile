NAME := grassroots_geocoder
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		:= debug

VPATH	:= \
	$(DIR_SRC) 
	

INCLUDES := \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	
SRCS 	:= \
	address.c \
	coordinate.c \
	country_codes.c \
	geocoder_util.c \
	google.c \
	nominatim.c
	

ifeq ($(BUILD),release)
	CFLAGS 	+= -O3 -s
 	LDFLAGS += -s
else
	CFLAGS 	+= -g
	CPPFLAGS += -D_DEBUG
endif
	

CPPFLAGS += -DGRASSROOTS_GEOCODER_LIBRARY_EXPORTS -DUNIX 

LDFLAGS += -ldl \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UUID_LIB) -l$(GRASSROOTS_UUID_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVER_LIB) -l$(GRASSROOTS_SERVER_LIB_NAME)

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile



