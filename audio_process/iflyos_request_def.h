#ifndef __IFLYOS_REQUEST_DEF_H__
#define __IFLYOS_REQUEST_DEF_H__

#include "config.h"

/********
 *  实现 recognizer, system, audio player三种定义
*********/


//recognizer
const char* recog_audion_in         = "recognizer.audio_in";
const char* recog_text_in           = "recognizer.text_in"; 
//system
const char* sys_state_sync          = "system.state_sync";
const char* sys_exception           = "system.exception";
const char* sys_sw_update_result    = "system.check_software_update_result";
const char* sys_update_sw_state     = "system.update_software_state_sync";
//audio player
const char* aplayer_pl_prog_sync    = "audio_player.playback.progress_sync";
const char* aplayer_tts_prog_sync   = "audio_player.tts.progress_sync";
const char* aplayer_ring_prog_sync  = "audio_player.ring.progress_sync";
const char* aplayer_tts_text_in     = "audio_player.tts.text_in";
//local alarm
const char* alarm_state_sync        = "alarm.state_sync";
//video player
const char* vplayer_prog_sync       = "vedio_player.progress_sync";
//playback controller
const char* pc_ctrl_cmd             = "playback_controller.control_command";
//app action
const char* app_check_result        = "app_action.check_result";
const char* app_exec_success        = "app_action.execute_succeed";
const char* app_exec_fail           = "app_action.execute_failed";
//template
const char* tmpl_elem_sel           = "template.element_selected";
//launcher
const char* launch_sa_result        = "launcher.start_activity_result";
const char* launch_back_result      = "launcher.back_result";
const char* launch_sel_result       = "launcher.select_result";
//wakeword
const char* ww_result               = "wakeword.set_wakeword_result";


#pragma pack(push, 1)
//recognizer
/******
 * proifle: CLOSE_TALK, FAR_FIELD, EVALUATE
 * format: AUDIO_L16_RATE_16000_CHANELS_1
 * category: read_chapter, read_sentence, read_word, read_syllable
 *****/
typedef struct iflyos_recog_ai_request
{
    char* request_name;
    char* request_id;
    char* reply_key;
    BOOL enable_vad;
    int vad_eos;
    char* profile;
    char* format;
    char* wake_up_word;
    int wake_up_score;
    int start_index;
    int end_index;
    char *prompt;
    char* language;
    char* category;
    char* text;
}FlyosRecogAiRequest;

typedef struct iflyos_recog_ti_request
{
    char* request_name;
    char* request_id;
    char* query;
    BOOL with_tts;
    char* reply_key;
}FlyosRecogTiRequest;

//system
typedef struct iflyos_sys_update_request
{
    char* request_name;
    char* request_id;
    char* result;
    BOOL need_update;
    char* ver_name;
    char* description;
}FlyosSysUpdateRequest;

typedef struct iflyos_sys_update_sync_request
{
    char* request_name;
    char* request_id;
    char* state;
    char* ver_name;
    char* description;
    char* type;
    char* message;
}FlyosSysUpdateSyncRequest;

typedef struct iflyos_sys_exception_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* code;
    char* message;
}FlyosSysExceptionRequest;

//audio player
typedef struct iflyos_audio_play_sync_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* resource_id;
    char* offset;
    long fail_code;
}FlyosAudioPlaySyncRequest;

typedef struct iflyos_audio_other_sync_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* resource_id;
}FlyosAudioOtherSyncRequest;

typedef struct iflyos_audio_tts_in_request
{
    char* request_name;
    char* request_id;
    char* text;
    int number;
    int volume;
    char* vcn;
}FlyosAudioTtsInRequest;
#pragma pack(pop)


#endif //__IFLYOS_REQUEST_DEF_H__