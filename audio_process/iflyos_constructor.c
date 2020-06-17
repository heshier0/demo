#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "config.h"

cJSON* iflyos_create_header(FlyosHeader* header)
{
    if (NULL == header)
    {
        return;
    }

    cJSON *root = NULL;
    cJSON *device = NULL;
    cJSON *location = NULL;
    cJSON *platform = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "authorization", cJSON_CreateString(header->authorization));
    
    cJSON_AddItemToObject(root, "device", device = cJSON_CreateObject());
    cJSON_AddStringToObject(device, "device_id", header->device_id);
    cJSON_AddStringToObject(device, "ip", header->device_ip);
    cJSON_AddItemToObject(device, "location", location = cJSON_CreateObject());
    cJSON_AddNumberToObject(location, "latitude", header->latitude);
    cJSON_AddNumberToObject(location, "longtitude", header->longitude);
    cJSON_AddItemToObject(device, "platform", platform = cJSON_CreateObject());
    cJSON_AddStringToObject(platform, "name", header->platform_name);
    cJSON_AddStringToObject(platform, "version", header->platform_version);

    return root;
}

void iflyos_create_context()
{

}

void iflyos_create_request()
{

}

void iflyos_create_protol()
{
    cJSON *root = NULL;

    //for test
    FlyosHeader *header = (FlyosHeader *)malloc(sizeof(FlyosHeader));
    memset(header, 0, sizeof(FlyosHeader));
    strcpy(header->authorization, "WKAeL95n1ZNgsxNOmHf0upvkmq58MJKgl30aqEhIkOZfL_IL9lMUbGprmSmGjY3A");
    strcpy(header->device_id, "HXT20200607P");
    strcpy(header->platform_name, "linux");
    strcpy(header->platform_version, "1.0.0");

    //end test
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "iflyos_header", iflyos_create_header(header));

    //for test
    char * fmt_json = cJSON_Print(root);
    if (fmt_json != NULL)
    {
        printf("%s\n", fmt_json);
    }
    //end test

    return;
}