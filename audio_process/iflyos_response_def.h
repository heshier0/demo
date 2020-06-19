#ifndef __IFLYOS_RESPONSE_DEF_H__
#define __IFLYOS_RESPONSE_DEF_H__

#include "config.h"

/********
 *  实现 recognizer, system, audio player三种定义
*********/

//recognizer
const char* recog_intermediate_text     = "recognizer.intermediate_text";
const char* recog_stop_capture          = "recognizer.stop_capture";
const char* recog_expect_reply          = "recognizer.expect_reply";
const char* recog_evaluate_result       = "evaluate_result";
//system
const char* sys_ping                    = "system.ping";
const char* sys_error                   = "system.error";
const char* sys_chk_sw_update           = "system.check_software_update";
const char* sys_update_sw               = "system.update_software";
const char* sys_power_off               = "system.power_off";
const char* sys_update_dev_modes        = "system.update_device_modes";
const char* sys_factory_rst             = "system.factory_reset";
const char* sys_reboot                  = "system.reboot";
const char* sys_revoke_response         = "system.revoke_response";
const char* sys_update_alarm_list       = "system.update_cloud_alarm_list";
//audio player
const char* aplayer_audio_out           = "audio_player.audio_out";
//local alarm
const char* alarm_set_alarm             = "alarm.set_alarm";
const char* alarm_delete_alarm          = "alarm.delete_alarm";
//speaker
const char* spk_set_volume              = "speaker.set_volume";
//video player
const char* vplayer_video_out           = "video_player.video_out";
//app action
const char* app_excute                  = "app_action.excute";
const char* app_check                   = "app_action.check";
//screen control
const char* screen_set_state            = "screen.set_state";
const char* screen_set_bright           = "screen.set_brightness";
//template
const char* tmpl_static                 = "template.static_template";
const char* tmpl_playing                = "template.playing_template";
const char* tmpl_custom                 = "template.custom_template";
const char* tmpl_exit                   = "template.exit";
//launcher
const char* launch_sa                   = "launcher.start_activity";
const char* launch_back                 = "launcher.back";
const char* launch_sel                  = "launcher.select";
//wake word
const char* ww_set_ww                   = "wakeword.set_wakeword";
//interceptor
const char* itcpt_custom                = "interceptor.custom";
const char* itcpt_trans_sema            = "interceptor.transfer_semantic";


#pragma pack(push,1)
//intermediate text
typedef struct iflyos_recog_it_response
{
    char* header;
    char* text;
    BOOL is_last;
}FlyosRecogITResponse;

//expect reply
typedef struct iflyos_recog_er_response
{
    char* header;
    char* reply_key;
    BOOL bg_recognize;
    long timeout;
}FlyosRecogERResponse;

//evaluate reply
typedef struct iflyos_recog_ev_response
{
    char* header;
    int code;
    char* description;
    char* sid;
    char* data;
}FlyosRecogEVRespons;

//ping
typedef struct iflyos_sys_ping_response
{
    char* header;
    long timestam;
}FlyosSysPingResponse;

//error
typedef struct iflyos_sys_error_response
{
    char* header;
    int code;      
    char* message;
}FlyosSysErrorResponse;

//udpate device modes
typedef struct iflyos_sys_modes_response
{
    char* header;
    BOOL kid;
    BOOL interaction;
}FlyosSysModesResponse;

//audio player
typedef struct iflyos_audio_out_response
{
    char* header;
    char* type;
    char* control;
    char* behavior;
    char* url;
    char* secure_url;
    char* resource_id;
    long offset;
    long duration;
    char* text; 
}FlyosAudioOutResponse;


#pragma pack(pop)


#endif //__IFLYOS_RESPONSE_DEF_H__