# Target platform
ifndef TARGET
	TARGET=srf06-cc26xx
	BOARD=sensortag/cc2650
endif

all: coap-server coap-client

	# Upload
upload: coap-server.upload
upload.server: coap-server.upload
upload.client: coap-client.upload

CONTIKI=../../../contiki

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
REST_RESOURCES_FILES = $(notdir $(shell bash -c "find $(REST_RESOURCES_DIR) -name '*.c'"))

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
