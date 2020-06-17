#include <stdlib.h>
#include <string.h>

typedef enum
{
    TRUE = 1, 
    FALSE = 0
} BOOL;

typedef struct iflyos_header
{
    char authorization[255];            //must
    char device_id[64];                 //must
    char device_ip[16];             
    float latitude;
    float longitude;
    char platform_name[16];             //must
    char platform_version[16];          //must
}FlyosHeader;

typedef struct iflyos_context_system
{
    char version[8];                    //must
    BOOL software_updater;
    BOOL power_controller;
    BOOL device_modes;
    BOOL factory_reset;
    BOOL reboot;
}FlyosContextSystem;

typedef struct iflyos_context_recognizer
{
    char version[8];
}FlyosContextRecognizer;



typedef struct iflyos_context
{
    FlyosContextSystem system;
    FlyosContextRecognizer recognizer;

}FlyosContext;